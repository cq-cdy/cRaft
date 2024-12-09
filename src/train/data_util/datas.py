from enum import Enum
import math
class State(Enum):
    FOLLOWER = 0
    CANDIDATE = 1
    LEADER = 2

class ActionType    (Enum):
    LOCAL = 0,
    REMOTE = 1,
class Action(Enum):
    CHANGE_STATE = 0
    RECEIVE_APPEND_ENTRIES = 1
    APPEND_ENTRIES = 2
    REQUEST_VOTE = 3
    RECEIVE_REQUEST_VOTE = 4
    SEND_INSTALL_SNAPSHOT_TO_PEER = 5
    RECEIVE_SNAPSHOT_FILE = 6
    TO_C=5
    TO_L=6
    TO_F=7
    CALL_0=8
    CALL_1=9
    CALL_2=10
    CALL_3=11
    CALL_4=12
    CORE_CUMP=13
    
    # for single server
    # F_TO_C = 7
    # C_TO_L = 8
    # L_TO_C = 9
    # C_TO_F = 10
    
    # # for center server
    # CS_LFFFF = 11
    # CS_FLFFF = 12
    # CS_FFLFF = 13
    # CS_FFFLF = 14
    # CS_FFFFL = 15
    # NOTHING = 16

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
    'to_c': Action.TO_C,
    'to_l': Action.TO_L,
    'to_f': Action.TO_F,
    # 'f_to_c': Action.F_TO_C,
    # 'c_to_l': Action.C_TO_L,
    # 'l_to_c': Action.L_TO_C,
    # 'c_to_f': Action.C_TO_F
}

def filterPredicate(key):
    return key in ["raft_state.ip_index","raft_state.nextIndex"]

def flattenJson(js_data, prefix='', filter_func=None):
    """
    展平 JSON 数据，返回键和值的两个列表，保持一一对应。
    
    :param js_data: 需要展平的 JSON 数据（字典）
    :param prefix: 用于构建复合键的前缀字符串，默认为空
    :param filter_func: 可选的过滤函数，接受一个键作为参数并返回布尔值
    :return: 两个列表，分别是展平后的键列表和值列表
    """
    keys = []
    values = []

    for key, value in js_data.items():
        if filter_func and filter_func(key):
            continue

        # 构建复合键
        full_key = f"{prefix}.{key}" if prefix else key

        if isinstance(value, dict):
            # 如果值是字典，则递归调用 flattenJson
            sub_keys, sub_values = flattenJson(value, full_key, filter_func)
            keys.extend(sub_keys)
            values.extend(sub_values)
        else:
            # 对特定键进行转换
            if key == 'timestamp':
                value = int(value)  # 确保可以安全转换
            elif key == 'state' and isinstance(state_to_int, dict) and value in state_to_int:
                value = state_to_int[value].value
            elif key == 'action' and isinstance(action_to_int, dict) and value in action_to_int:
                value = action_to_int[value].value
            elif key == 'cpu':
                value = float(value) * 10  # 确保可以安全转换

            # 添加键值对到列表中
            keys.append(full_key)
            values.append(value)

    return keys, values

# def flattenJson(json):
#     keys = flattenJsonKeys(json)
#     values = flattenJsonValue(json)
#     js_data = {}
#     for i in range(len(keys)):
#         js_data[keys[i]] = values[i]
#     return js_data

def handleOriginFlattendJson(js):
    if('timestamp' in js):js.pop('timestamp')
    if('role' in js):js.pop('role')
    if('raft_state.commitIndex' in js):js.pop('raft_state.commitIndex')
    if('raft_state.lastLogIndex' in js):js.pop('raft_state.lastLogIndex')
    if('raft_state.logsize' in js):js.pop('raft_state.logsize')
    if('raft_state.snapShotIndex' in js):js.pop('raft_state.snapShotIndex')
    if('raft_state.snapShotTerm' in js):js.pop('raft_state.snapShotTerm')
    if('raft_state.term' in js):js.pop('raft_state.term')
    for key in js:
        if js[key] is None:
            js[key] = 0
        if(key.startswith('system_state')):
            js[key] = math.log10(1 + js[key])
    return js


def handleStateJson(js):
    js = flattenJson(js)
    return handleOriginFlattendJson(js)

def average_k_elements(lst, k):
    if k <= 0:
        raise ValueError("K must be a positive integer greater than zero.")
    averages = []
    for i in range(0, len(lst), k):
        group = lst[i:i+k]  
        group_average = sum(group) / len(group)  
        averages.append(group_average)  
    return averages