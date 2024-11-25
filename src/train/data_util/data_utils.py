import json
import copy
from torch.utils.data import Dataset

LEADER = 0
FOLLOWER = 1
CANDIDATE = 2

state_to_int = {
    'LEADER':LEADER,
    'FOLLOWER':FOLLOWER,
    'CANDIDATE':CANDIDATE
}


CHANGE_STATE = 0
RECEIVE_APPEND_ENTRIES = 1
APPEND_ENTRIES = 2
REQUEST_VOTE = 3
RECEIVE_REQUEST_VOTE = 4
SEND_INSTALL_SNAPSHOT_TO_PEER = 5
RECEIVE_SNAPSHOT_FILE = 6

action_to_int = {
    'change_state' :CHANGE_STATE,
    'receive_append_entries' :RECEIVE_APPEND_ENTRIES,
    'appendEntries':  APPEND_ENTRIES, 
    'requestVote': REQUEST_VOTE, 
    'receive_request_vote': RECEIVE_REQUEST_VOTE,
    'sendInstallSnapshotToPeer':  SEND_INSTALL_SNAPSHOT_TO_PEER,
    'receive_snapshot_file': RECEIVE_SNAPSHOT_FILE
}

def filterPredicate(key):
    return key in ["ip_index","nextIndex"]

def flattenJsonKeys(js_data):
    keys = []
    for key in js_data:
        if filterPredicate(key):continue
        if isinstance(js_data[key], dict) :
            keys.extend(flattenJsonKeys(js_data[key]))
        else:
            keys.append(key)
    return keys
    
def flattenJsonValue(js_data):
    values = []
    for key in js_data:
        if filterPredicate(key):continue
        if isinstance(js_data[key], dict):
            values.extend(flattenJsonValue(js_data[key]))
        else:
            v = js_data[key]
            if (key == 'timestamp'):
                values.append(int(v))
            elif key == 'state':
                values.append(state_to_int[v])
            elif key == 'action':
                values.append(action_to_int[v])
            elif key == 'cpu':
                values.append(v * 10)
            else:
                values.append(v)
    return values


class RaftLogDataSet(Dataset):
    def __init__(self, datafilesName):
        self.datafilesName = datafilesName
        self.serversOrignDatas = []
        for fileName in datafilesName:
           with open(fileName, 'r') as f:
                lines = f.readlines()
                tmp = []
                for line in lines:
                    if len(line) < 2:continue
                    js_data = json.loads(line)
                    tmp.append(js_data)
                self.serversOrignDatas.append(tmp)
                f.close()
                
        self.all_servers_datasets = []
        for serverOrignData in self.serversOrignDatas:
            flattend_jsons = []
            for js_data in serverOrignData:
                if ('role' not in js_data) or (line is None) or ('timestamp' not in js_data):
                    continue
              
                keys = flattenJsonKeys(js_data)
                values = flattenJsonValue(js_data)
                js_data = {}
                for i in range(len(keys)):
                    js_data[keys[i]] = values[i]
                flattend_jsons.append(js_data)
            self.all_servers_datasets.append(flattend_jsons)
            
        combined_actionTuples = []
        '''
        combined_actionTuples :[ (timestamp,[(serverId,actionJson,index) ... ]) ... ]
        '''
        def timestampIsExistInTuples(combined_actionTuples,timeStamp) -> int:
            for i in range(len(combined_actionTuples)):
                if combined_actionTuples[i][0] == timeStamp:
                    return i
            return -1
        
        def getPrevStateJsonInServerByIndex(serverId,actionIndex):
            serverDatas = self.all_servers_datasets[serverId]
            for i in range(actionIndex,-1,-1):
                if serverDatas[i]['role'] == 'state':
                    return serverDatas[i]
            return None

        def getNextStateJsonInServerByIndex(serverId,actionIndex):
            serverDatas = self.all_servers_datasets[serverId]
            for i in range(actionIndex,len(serverDatas)):
                if serverDatas[i]['role'] == 'state':
                    return serverDatas[i]
            return None
        
        for serverId in range(len(self.all_servers_datasets)):
            serverDatas = self.all_servers_datasets[serverId]
            for i  in range(len(serverDatas)):
                json_data = serverDatas[i]
                if(json_data['role'] != 'action'):continue
                timestamp = json_data['timestamp']
                idx = timestampIsExistInTuples(combined_actionTuples,timestamp)
                tuple_ = (serverId,json_data,i)
                if idx == -1:
                    combined_actionTuples.append((timestamp,[tuple_]))
                else:   
                    combined_actionTuples[idx][1].append(tuple_)
                    
        self.train_data_each_batch_by_logicOrder = []
        for tp_ in combined_actionTuples:
            # tp_ : (timestamp,[(serverId,actionJson,index) ... ])
            actionTupleList = tp_[-1]
            grouped_by_serverId = {}
            for actionTuple in actionTupleList:
                # actionTuple : (serverId,actionJson,index)
                serverId = actionTuple[0]
                if serverId not in grouped_by_serverId:
                    grouped_by_serverId[serverId] = []
                grouped_by_serverId[serverId].append(actionTuple)
                
            batch_tuple = {}
            for serverId in grouped_by_serverId:
                batch_tuple[serverId] = {
                    'prevStateJson' : None,
                    'actions' : [],
                    'nextStateJson':None
                }
                actionTuples = grouped_by_serverId[serverId]
                prevStateJson = None
                nextStateJson = None
                # actionTuple: (serverId,actionJson,index)
                for actionTuple in actionTuples:
                    actionJson = actionTuple[1]
                    index = actionTuple[2]
                    if prevStateJson is None:
                        prevStateJson = getPrevStateJsonInServerByIndex(serverId,index)
                    if nextStateJson is None:
                        nextStateJson = getNextStateJsonInServerByIndex(serverId,index)
                    batch_tuple[serverId]['actions'].append(actionJson)
                batch_tuple[serverId]['prevStateJson'] = prevStateJson
                batch_tuple[serverId]['nextStateJson'] = nextStateJson
            self.train_data_each_batch_by_logicOrder.append(batch_tuple)
          

    def __getitem__(self, index):
        
        if(index >= len(self.train_data_each_batch_by_logicOrder) or index < 0):
            raise IndexError("Index out of range")
        batch_tuple = self.train_data_each_batch_by_logicOrder[index]
        once_prevStateJson = []
        once_actions = []
        once_nextStateJson = []
        for serverId in batch_tuple:
            item = batch_tuple[serverId]
            prevStateJson = item['prevStateJson']
            nextStateJson = item['nextStateJson']
            actions = item['actions']
            once_prevStateJson.append(prevStateJson)
            once_nextStateJson.append(nextStateJson)
            once_actions.append(actions)  
        return once_prevStateJson,once_actions,once_nextStateJson  
    
    def __len__(self):
        return len(self.train_data_each_batch_by_logicOrder)
    
# datafilesName = ['/home/cdy/code/projects/cRaft/.data/system_data/server-128-0/system_runtime.data0',
#                      '/home/cdy/code/projects/cRaft/.data/system_data/server-130-1/system_runtime.data0',
#                      '/home/cdy/code/projects/cRaft/.data/system_data/server-131-2/system_runtime.data0',
#                      '/home/cdy/code/projects/cRaft/.data/system_data/server-133-3/system_runtime.data0',
#                      '/home/cdy/code/projects/cRaft/.data/system_data/server-134-4/system_runtime.data0',
#                 ]
# dataset = RaftLogDataSet(datafilesName)