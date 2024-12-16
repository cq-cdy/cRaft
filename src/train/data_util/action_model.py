import torch
import torch.nn as nn
import torch.nn.functional as F

class CenterActionNetwork(nn.Module):
    def __init__(self, num_servers, action_nums, input_size, hidden_size=512, num_attention_heads=4):
        super(CenterActionNetwork, self).__init__()
        self.num_servers = num_servers
        self.action_nums = action_nums
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.num_attention_heads = num_attention_heads
        
        # Input Linear Layer
        self.input_layer = nn.Linear(input_size, hidden_size)
        self.input_norm = nn.LayerNorm(hidden_size)  # LayerNorm after input layer
        
        # Shared Fully Connected Layers
        self.shared_fc = nn.Sequential(
            nn.Linear(hidden_size, hidden_size),
            nn.LayerNorm(hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, hidden_size),
            nn.LayerNorm(hidden_size),
            nn.ReLU()
        )
        
        # Multi-Head Attention with LayerNorm
        self.attention = nn.MultiheadAttention(embed_dim=hidden_size, num_heads=num_attention_heads, batch_first=True)
        self.attention_norm = nn.LayerNorm(hidden_size)  # LayerNorm after residual connection
        
        # Server-Specific Fully Connected Layers
        self.server_fc1 = nn.ModuleList([
            nn.Sequential(
                nn.Linear(hidden_size // num_servers, hidden_size // num_servers),
                nn.LayerNorm(hidden_size // num_servers),
                nn.ReLU(),
                nn.Linear(hidden_size // num_servers, action_nums)
            ) for _ in range(num_servers)
        ])
        
    def forward(self, x):
        """
        x: Tensor of shape (batch_size, num_servers, input_size)
        """
        batch_size, num_servers, _ = x.shape
        
        # Step 1: Input layer and normalization
        x = self.input_layer(x)  # Linear layer
        x = self.input_norm(x)   # LayerNorm
        x = F.relu(x)
        
        # Step 2: Shared fully connected layers
        x = self.shared_fc(x)    # Shared FC layers
        x = x.view(batch_size, num_servers, self.hidden_size)
        
        # Step 3: Multi-head Attention with Residual Connection
        attn_output, _ = self.attention(x, x, x)  # Self-attention
        x = x + attn_output        # Residual connection
        x = self.attention_norm(x) # LayerNorm after residual
        x = F.relu(x)
        
        # Step 4: Split hidden states and apply server-specific layers
        split_size = self.hidden_size // self.num_servers
        outputs = []
        
        for i in range(self.num_servers):
            hidden_part = x[:, i, :split_size]  # Split the hidden representation for each server
            output = self.server_fc1[i](hidden_part)  # Pass through server-specific FC layers
            outputs.append(output.unsqueeze(1))
        
        # Concatenate outputs for all servers
        outputs = torch.cat(outputs, dim=1)  # Shape: (batch_size, num_servers, action_nums)
        return outputs,attn_output
    
class SingleActionNetWork(torch.nn.Module):
    def __init__(self, state_critic_model,acion_nums, input_size, hidden_size=512, num_attion_head=8):
        super(SingleActionNetWork, self).__init__()
        
        self.state_critic_model = state_critic_model
        self.state_critic_model.eval()
        self.action_nums = acion_nums 
        self.input_size = input_size
        self.hidden_size = hidden_size
        self.num_attion_head = num_attion_head

    def forward(self, x):
        # x :(batch_size, input_size)
        state_value = self.state_critic_model(x).item()
        # state_value 越大，越需要尽可能的交付日志，若交付失败，则可以告诉客户端，就可能集群即将重新选主
        # state_value 越小，就可以尽可能的减少交付日志，减少网络开销
        # 仍存在有未被同步的日志被覆盖的情况。
        return x
        