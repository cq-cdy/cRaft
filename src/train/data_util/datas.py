from enum import Enum

class State(Enum):
    FOLLOWER = 0
    CANDIDATE = 1
    LEADER = 2
        
class Action(Enum):
    CHANGE_STATE = 0
    RECEIVE_APPEND_ENTRIES = 1
    APPEND_ENTRIES = 2
    REQUEST_VOTE = 3
    RECEIVE_REQUEST_VOTE = 4
    SEND_INSTALL_SNAPSHOT_TO_PEER = 5
    RECEIVE_SNAPSHOT_FILE = 6
    
    # for single server
    F_TO_C = 7
    C_TO_L = 8
    L_TO_C = 9
    C_TO_F = 10
    
    # for center server
    CS_LFFFF = 11
    CS_FLFFF = 12
    CS_FFLFF = 13
    CS_FFFLF = 14
    CS_FFFFL = 15

state_to_int = {
    'LEADER':State.LEADER,
    'FOLLOWER':State.FOLLOWER,
    'CANDIDATE':State.CANDIDATE
}

action_to_int = {
    'change_state' :Action.CHANGE_STATE,
    'receive_append_entries' :Action.RECEIVE_APPEND_ENTRIES,
    'appendEntries':  Action.APPEND_ENTRIES, 
    'requestVote': Action.REQUEST_VOTE, 
    'receive_request_vote': Action.RECEIVE_REQUEST_VOTE,
    'sendInstallSnapshotToPeer':  Action.SEND_INSTALL_SNAPSHOT_TO_PEER,
    'receive_snapshot_file': Action.RECEIVE_SNAPSHOT_FILE,
    'f_to_c': Action.F_TO_C,
    'c_to_l': Action.C_TO_L,
    'l_to_c': Action.L_TO_C,
    'c_to_f': Action.C_TO_F
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

def flattenJson(json):
    keys = flattenJsonKeys(json)
    values = flattenJsonValue(json)
    js_data = {}
    for i in range(len(keys)):
        js_data[keys[i]] = values[i]
    return js_data

def handleOriginFlattendJson(js):
    if('timestamp' in js):js.pop('timestamp')
    if('role' in js):js.pop('role')
    if('commitIndex' in js):js.pop('commitIndex')
    if('lastLogIndex' in js):js.pop('lastLogIndex')
    if('logsize' in js):js.pop('logsize')
    if('snapShotIndex' in js):js.pop('snapShotIndex')
    if('snapShotTerm' in js):js.pop('snapShotTerm')
    if('term' in js):js.pop('term')
    for key in js:
        if js[key] is None:
            js[key] = 0
        if(js[key] > 1024):
            js[key] = js[key] / 1024
    js['TX Bytes'] = js['TX Bytes'] / 1024
    js['RX Bytes'] = js['RX Bytes'] / 1024
    return js

def CenterServerOriginFlattendJson(js):
    if('timestamp' in js):js.pop('timestamp')
    for key in js:
        if js[key] is None:
            js[key] = 0
        if(js[key] > 1024):
            js[key] = js[key] / 1024
    js['TX Bytes'] = js['TX Bytes'] / 1024
    js['RX Bytes'] = js['RX Bytes'] / 1024
    return js

def handleStateJson(js):
    js = flattenJson(js)
    return handleOriginFlattendJson(js)