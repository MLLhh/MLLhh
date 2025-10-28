import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F

__all__ = ['g_ghost_regnetx_002', 'g_ghost_regnetx_004', 'g_ghost_regnetx_006', 'g_ghost_regnetx_008',
           'g_ghost_regnetx_016', 'g_ghost_regnetx_032',
           'g_ghost_regnetx_040', 'g_ghost_regnetx_064', 'g_ghost_regnetx_080', 'g_ghost_regnetx_120',
           'g_ghost_regnetx_160', 'g_ghost_regnetx_320']


def conv3x3(in_planes, out_planes, stride=1, groups=1, dilation=1):
    """3x3 convolution with padding"""
    return nn.Conv2d(in_planes, out_planes, kernel_size=3, stride=stride,
                     padding=dilation, groups=groups, bias=False, dilation=dilation)


def conv1x1(in_planes, out_planes, stride=1):
    """1x1 convolution"""
    return nn.Conv2d(in_planes, out_planes, kernel_size=1, stride=stride, bias=False)


class SEModule(nn.Module):
    def __init__(self, channels, reduction=16):
        super(SEModule, self).__init__()
        self.avg_pool = nn.AdaptiveAvgPool2d(1)
        self.fc = nn.Sequential(
            nn.Linear(channels, channels // reduction, bias=False),
            nn.ReLU(inplace=True),
            nn.Linear(channels // reduction, channels, bias=False),
            nn.Sigmoid()
        )

    def forward(self, x):
        b, c, _, _ = x.size()
        y = self.avg_pool(x).view(b, c)
        y = self.fc(y).view(b, c, 1, 1)
        return x * y


class DFCAttention(nn.Module):
    def __init__(self, in_channels, reduction=16):
        super(DFCAttention, self).__init__()
        self.conv1 = nn.Conv2d(in_channels, in_channels // reduction, kernel_size=1)
        self.conv2 = nn.Conv2d(in_channels // reduction, in_channels, kernel_size=1)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        avg_out = F.adaptive_avg_pool2d(x, 1)  # 全局平均池化
        out = self.conv1(avg_out)
        out = F.relu(out, inplace=True)
        out = self.conv2(out)
        scale = self.sigmoid(out)
        return x * scale


class ChannelAttention(nn.Module):
    def __init__(self, in_planes, ratio=16):
        super(ChannelAttention, self).__init__()
        self.avg_pool = nn.AdaptiveAvgPool2d(1)
        self.max_pool = nn.AdaptiveMaxPool2d(1)

        self.shared_MLP = nn.Sequential(
            nn.Conv2d(in_planes, in_planes // ratio, 1, bias=False),
            nn.ReLU(),
            nn.Conv2d(in_planes // ratio, in_planes, 1, bias=False)
        )
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        avg_out = self.shared_MLP(self.avg_pool(x))
        max_out = self.shared_MLP(self.max_pool(x))
        out = avg_out + max_out
        return self.sigmoid(out)


class SpatialAttention(nn.Module):
    def __init__(self, kernel_size=3):
        super(SpatialAttention, self).__init__()
        padding = (kernel_size - 1) // 2

        self.conv = nn.Conv2d(2, 1, kernel_size=kernel_size, padding=padding, bias=False)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        avg_out = torch.mean(x, dim=1, keepdim=True)
        max_out, _ = torch.max(x, dim=1, keepdim=True)
        x_cat = torch.cat([avg_out, max_out], dim=1)
        return self.sigmoid(self.conv(x_cat))


class CBAM(nn.Module):
    def __init__(self, in_planes, ratio=16, kernel_size=3):
        super(CBAM, self).__init__()
        self.channel_attention = ChannelAttention(in_planes, ratio)
        self.spatial_attention = SpatialAttention(kernel_size)

    def forward(self, x):
        x = x * self.channel_attention(x)
        x = x * self.spatial_attention(x)
        return x


# ------------------ Coordinate Attention (added) ------------------
class CoordinateAttention(nn.Module):
    """
    Coordinate Attention implementation.
    Keeps lightweight while embedding positional information via separate
    height and width pooling, following the paper "Coordinate Attention for Efficient Mobile Network Design".
    """

    def __init__(self, in_channels, reduction=32):
        super(CoordinateAttention, self).__init__()
        self.in_channels = in_channels
        self.reduction = reduction
        mid_channels = max(8, in_channels // reduction)

        # shared transform
        self.conv1 = nn.Conv2d(in_channels, mid_channels, kernel_size=1, stride=1, padding=0, bias=False)
        self.bn1 = nn.BatchNorm2d(mid_channels)
        self.act = nn.ReLU(inplace=True)

        # separate transforms for H and W
        self.conv_h = nn.Conv2d(mid_channels, in_channels, kernel_size=1, stride=1, padding=0, bias=False)
        self.conv_w = nn.Conv2d(mid_channels, in_channels, kernel_size=1, stride=1, padding=0, bias=False)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        # x: B, C, H, W
        b, c, h, w = x.size()
        # Pool along width and height respectively
        x_h = torch.mean(x, dim=3, keepdim=True)  # B, C, H, 1
        x_w = torch.mean(x, dim=2, keepdim=True)  # B, C, 1, W

        # transpose x_w to shape B, C, W, 1 so we can concat along spatial dim
        x_w = x_w.permute(0, 1, 3, 2)  # B, C, W, 1

        # concat along spatial dimension (height-like)
        y = torch.cat([x_h, x_w], dim=2)  # B, C, H+W, 1

        # shared 1x1 conv
        y = self.conv1(y)  # B, mid, H+W, 1
        y = self.bn1(y)
        y = self.act(y)

        # split
        y_h, y_w = torch.split(y, [h, w], dim=2)  # y_h: B, mid, H,1 ; y_w: B, mid, W,1

        # restore y_w shape to (B, mid, 1, W)
        y_w = y_w.permute(0, 1, 3, 2)  # B, mid, 1, W

        # apply separate convs
        a_h = self.conv_h(y_h).sigmoid()  # B, C, H, 1
        a_w = self.conv_w(y_w).sigmoid()  # B, C, 1, W

        # broadcast multiply
        out = x * a_h * a_w
        return out


# -----------------------------------------------------------------

class Bottleneck(nn.Module):
    expansion = 1
    __constants__ = ['downsample']

    def __init__(self, inplanes, planes, stride=1, downsample=None, group_width=1,
                 dilation=1, norm_layer=None):
        super(Bottleneck, self).__init__()
        if norm_layer is None:
            norm_layer = nn.BatchNorm2d
        width = planes * self.expansion
        # Both self.conv2 and self.downsample layers downsample the input when stride != 1
        self.conv1 = conv1x1(inplanes, width)
        self.bn1 = norm_layer(width)
        self.conv2 = conv3x3(width, width, stride, width // min(width, group_width), dilation)
        self.bn2 = norm_layer(width)
        self.conv3 = conv1x1(width, planes)
        self.bn3 = norm_layer(planes)
        # self.se = SEModule(planes)SE注意力机制
        # self.attn = DFCAttention(planes)DFC注意力机制
        # replace CBAM with Coordinate Attention for lighter-weight positional-aware attention
        self.attn = CoordinateAttention(planes)

        self.relu = nn.ReLU(inplace=True)
        self.downsample = downsample
        self.stride = stride

    def forward(self, x):
        identity = x

        out = self.conv1(x)
        out = self.bn1(out)
        out = self.relu(out)

        out = self.conv2(out)
        out = self.bn2(out)
        out = self.relu(out)

        out = self.conv3(out)
        out = self.bn3(out)

        # out = self.se(out)SE注意力机制
        out = self.attn(out)  # 应用 Coordinate Attention

        if self.downsample is not None:
            identity = self.downsample(x)

        out += identity
        out = self.relu(out)

        return out


def slice_planes(x, raw_planes):
    return x[:, :raw_planes]


class LambdaLayer(nn.Module):
    def __init__(self, lambd):
        super(LambdaLayer, self).__init__()
        self.lambd = lambd

    def forward(self, x):
        return self.lambd(x)


class Stage(nn.Module):

    def __init__(self, block, inplanes, planes, group_width, blocks, stride=1, dilate=False, cheap_ratio=0.5):
        super(Stage, self).__init__()
        norm_layer = nn.BatchNorm2d
        downsample = None
        self.dilation = 1
        previous_dilation = self.dilation
        if dilate:
            self.dilation *= stride
            stride = 1
        if stride != 1 or inplanes != planes:
            downsample = nn.Sequential(
                conv1x1(inplanes, planes, stride),
                norm_layer(planes),
            )

        self.base = block(inplanes, planes, stride, downsample, group_width,
                          previous_dilation, norm_layer)
        self.end = block(planes, planes, group_width=group_width,
                         dilation=self.dilation,
                         norm_layer=norm_layer)

        group_width = int(group_width * 0.75)
        raw_planes = int(planes * (1 - cheap_ratio) / group_width) * group_width
        cheap_planes = planes - raw_planes
        self.cheap_planes = cheap_planes
        self.raw_planes = raw_planes

        self.merge = nn.Sequential(
            nn.AdaptiveAvgPool2d(1),
            nn.Conv2d(planes + raw_planes * (blocks - 2), cheap_planes,
                      kernel_size=1, stride=1, bias=False),
            nn.BatchNorm2d(cheap_planes),
            nn.ReLU(inplace=True),
            nn.Conv2d(cheap_planes, cheap_planes, kernel_size=1, bias=False),
            nn.BatchNorm2d(cheap_planes),
        )
        self.cheap = nn.Sequential(
            nn.Conv2d(cheap_planes, cheap_planes,
                      kernel_size=1, stride=1, bias=False),
            nn.BatchNorm2d(cheap_planes),
        )
        self.cheap_relu = nn.ReLU(inplace=True)

        layers = []
        downsample = nn.Sequential(
            LambdaLayer(self.slice_planes)
            # LambdaLayer(lambda x: x[:, :raw_planes])
        )

        layers = []
        layers.append(block(raw_planes, raw_planes, 1, downsample, group_width,
                            self.dilation, norm_layer))
        inplanes = raw_planes
        for _ in range(2, blocks - 1):
            layers.append(block(inplanes, raw_planes, group_width=group_width,
                                dilation=self.dilation,
                                norm_layer=norm_layer))

        self.layers = nn.Sequential(*layers)

    def slice_planes(self, x):
        return x[:, :self.raw_planes]

    def forward(self, input):
        x0 = self.base(input)

        m_list = [x0]
        e = x0[:, :self.raw_planes]
        for l in self.layers:
            e = l(e)
            m_list.append(e)
        m = torch.cat(m_list, 1)
        m = self.merge(m)

        c = x0[:, self.raw_planes:]
        c = self.cheap_relu(self.cheap(c) + m)

        x = torch.cat((e, c), 1)
        x = self.end(x)
        return x


class GGhostRegNet(nn.Module):

    def __init__(self, block, layers, widths, num_classes=1000, zero_init_residual=True,
                 group_width=1, replace_stride_with_dilation=None,
                 norm_layer=None):
        super(GGhostRegNet, self).__init__()
        if norm_layer is None:
            norm_layer = nn.BatchNorm2d
        self._norm_layer = norm_layer

        self.inplanes = 32
        self.dilation = 1
        if replace_stride_with_dilation is None:
            # each element in the tuple indicates if we should replace
            # the 2x2 stride with a dilated convolution instead
            replace_stride_with_dilation = [False, False, False, False]
        if len(replace_stride_with_dilation) != 4:
            raise ValueError("replace_stride_with_dilation should be None "
                             "or a 4-element tuple, got {}".format(replace_stride_with_dilation))
        self.group_width = group_width
        self.conv1 = nn.Conv2d(3, self.inplanes, kernel_size=3, stride=2, padding=1,
                               bias=False)
        self.bn1 = norm_layer(self.inplanes)
        self.relu = nn.ReLU(inplace=True)

        self.layer1 = self._make_layer(block, widths[0], layers[0], stride=2,
                                       dilate=replace_stride_with_dilation[0])

        self.inplanes = widths[0]
        if layers[1] > 2:
            self.layer2 = Stage(block, self.inplanes, widths[1], group_width, layers[1], stride=2,
                                dilate=replace_stride_with_dilation[1], cheap_ratio=0.5)
        else:
            self.layer2 = self._make_layer(block, widths[1], layers[1], stride=2,
                                           dilate=replace_stride_with_dilation[1])

        self.inplanes = widths[1]
        self.layer3 = Stage(block, self.inplanes, widths[2], group_width, layers[2], stride=2,
                            dilate=replace_stride_with_dilation[2], cheap_ratio=0.5)

        self.inplanes = widths[2]
        if layers[3] > 2:
            self.layer4 = Stage(block, self.inplanes, widths[3], group_width, layers[3], stride=2,
                                dilate=replace_stride_with_dilation[3], cheap_ratio=0.5)
        else:
            self.layer4 = self._make_layer(block, widths[3], layers[3], stride=2,
                                           dilate=replace_stride_with_dilation[3])
        self.avgpool = nn.AdaptiveAvgPool2d((1, 1))
        self.dropout = nn.Dropout(0.2)
        self.fc = nn.Linear(widths[-1] * block.expansion, num_classes)

        for m in self.modules():
            if isinstance(m, nn.Conv2d):
                nn.init.kaiming_normal_(m.weight, mode='fan_out', nonlinearity='relu')
            elif isinstance(m, (nn.BatchNorm2d, nn.GroupNorm)):
                nn.init.constant_(m.weight, 1)
                nn.init.constant_(m.bias, 0)

    def _make_layer(self, block, planes, blocks, stride=1, dilate=False):
        norm_layer = self._norm_layer
        downsample = None
        previous_dilation = self.dilation
        if dilate:
            self.dilation *= stride
            stride = 1
        if stride != 1 or self.inplanes != planes:
            downsample = nn.Sequential(
                conv1x1(self.inplanes, planes, stride),
                norm_layer(planes),
            )

        layers = []
        layers.append(block(self.inplanes, planes, stride, downsample, self.group_width,
                            previous_dilation, norm_layer))
        self.inplanes = planes
        for _ in range(1, blocks):
            layers.append(block(self.inplanes, planes, group_width=self.group_width,
                                dilation=self.dilation,
                                norm_layer=norm_layer))

        return nn.Sequential(*layers)

    def _forward_impl(self, x):
        # See note [TorchScript super()]
        x = self.conv1(x)
        x = self.bn1(x)
        x = self.relu(x)

        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = self.layer4(x)

        x = self.avgpool(x)
        x = torch.flatten(x, 1)
        x = self.dropout(x)
        x = self.fc(x)

        return x

    def forward(self, x):
        return self._forward_impl(x)


def g_ghost_regnetx_002(**kwargs):
    return GGhostRegNet(Bottleneck, [1, 1, 4, 7], [24, 56, 152, 368], group_width=8, **kwargs)


def g_ghost_regnetx_004(**kwargs):
    return GGhostRegNet(Bottleneck, [1, 2, 7, 12], [32, 64, 160, 384], group_width=16, **kwargs)


def g_ghost_regnetx_006(**kwargs):
    return GGhostRegNet(Bottleneck, [1, 3, 5, 7], [48, 96, 240, 528], group_width=24, **kwargs)


def g_ghost_regnetx_008(**kwargs):
    return GGhostRegNet(Bottleneck, [1, 3, 7, 5], [64, 128, 288, 672], group_width=16, **kwargs)


def g_ghost_regnetx_016(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 4, 10, 2], [72, 168, 408, 912], group_width=24, **kwargs)


def g_ghost_regnetx_032(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 6, 15, 2], [96, 192, 432, 1008], group_width=48, **kwargs)


def g_ghost_regnetx_040(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 5, 14, 2], [80, 240, 560, 1360], group_width=40, **kwargs)


def g_ghost_regnetx_064(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 4, 10, 1], [168, 392, 784, 1624], group_width=56, **kwargs)


def g_ghost_regnetx_080(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 5, 15, 1], [80, 240, 720, 1920], group_width=120, **kwargs)


def g_ghost_regnetx_120(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 5, 11, 1], [224, 448, 896, 2240], group_width=112, **kwargs)


def g_ghost_regnetx_160(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 6, 13, 1], [256, 512, 896, 2048], group_width=128, **kwargs)


def g_ghost_regnetx_320(**kwargs):
    return GGhostRegNet(Bottleneck, [2, 7, 13, 1], [336, 672, 1344, 2520], group_width=168, **kwargs)


if __name__ == '__main__':
    x = torch.randn(1, 3, 224, 224)
    model = g_ghost_regnetx_160(num_classes=6)
    out = model(x)
    print(out.shape)
