FOLLOWER = 0
CANDIDATE = 1
LEADER = 2
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
F_TO_C = 7
C_TO_L = 8
L_TO_C = 9
C_TO_F = 10

action_to_int = {
    'change_state' :CHANGE_STATE,
    'receive_append_entries' :RECEIVE_APPEND_ENTRIES,
    'appendEntries':  APPEND_ENTRIES, 
    'requestVote': REQUEST_VOTE, 
    'receive_request_vote': RECEIVE_REQUEST_VOTE,
    'sendInstallSnapshotToPeer':  SEND_INSTALL_SNAPSHOT_TO_PEER,
    'receive_snapshot_file': RECEIVE_SNAPSHOT_FILE,
    'f_to_c': F_TO_C,
    'c_to_l': C_TO_L,
    'l_to_c': L_TO_C,
    'c_to_f': C_TO_F
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
#reward  = -log(1 + timeline) * stata
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
def handleStateJson(js):
    js = flattenJson(js)
    return handleOriginFlattendJson(js)