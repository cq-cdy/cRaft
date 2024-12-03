import torch
import torch.nn as nn

class CenterActionNetWork(torch.nn.Module):
    def __init__(self, num_servers ,acion_nums, input_size, hidden_size=1024, num_attion_head=8):
        super(CenterActionNetWork, self).__init__()
        self.num_servers = num_servers
        self.input_size = input_size *  self.num_servers
        self.hidden_size = hidden_size
        self.num_attion_head = num_attion_head
        self.acion_nums = acion_nums

        self.sum_hidden_size = hidden_size *  self.num_servers
        self.liner1 = nn.Linear(self.input_size, self.hidden_size)
        self.liner2 = nn.Linear(self.hidden_size, self.sum_hidden_size)
        self.liner3 = nn.Linear(self.sum_hidden_size, self.sum_hidden_size)

        self.attenion_layer = nn.MultiheadAttention(self.sum_hidden_size, self.num_attion_head)

        self.liner4 = nn.Linear(self.sum_hidden_size, self.sum_hidden_size)

        self.each_output_linear_layers = nn.ModuleList(
            [nn.Linear(self.hidden_size, self.hidden_size) for _ in range(self.num_servers)]
        )
        self.each_output_action_learys = nn.ModuleList(
            [nn.Linear(self.hidden_size, self.acion_nums) for _ in range(self.num_servers)]
        )

        self.softmax = nn.Softmax(dim=-1)
        self.relu = nn.ReLU()
        self.dropout = nn.Dropout(0.3)

    def forward(self, x):
        x = x.view(2,-1)
        x = self.relu(self.liner1(x))
        x = self.dropout(x)

        x = self.relu(self.liner2(x))
        x = self.dropout(x)

        x = self.relu(self.liner3(x))
        x = self.dropout(x)
        residual = x
        attn_input =x.unsqueeze(1).transpose(0, 1) 
        attn_output, _ = self.attenion_layer(attn_input, attn_input, attn_input)
        attn_output = attn_output.transpose(0, 1).squeeze(1) 
        x = attn_output + 0.5 * residual
        x = self.relu(self.liner4(x))
        x = self.dropout(x)
        x_split = torch.split(x, self.hidden_size, dim=-1)
        outputs = []
        for i in range(self.num_servers):
            out = self.relu(self.each_output_linear_layers[i](x_split[i]))
            out = self.softmax(self.each_output_action_learys[i](out))
            outputs.append(out)
   

        outputs = torch.stack(outputs,dim=-2)
        return outputs