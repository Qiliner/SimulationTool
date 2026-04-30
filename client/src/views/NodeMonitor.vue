<template>
  <div class="monitor-container">
    <!-- 动态背景装饰 -->
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 顶部标题栏 -->
    <div class="header glass-panel">
      <div class="header-left">
        <!-- 添加返回按钮 -->
        <div class="back-icon" @click="goBack">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M19 12H5M12 19l-7-7 7-7"/>
          </svg>
        </div>
        <div class="logo-icon">
          <svg width="28" height="28" viewBox="0 0 24 24" fill="none">
            <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.5"/>
            <path d="M2 17L12 22L22 17" stroke="currentColor" stroke-width="1.5"/>
            <path d="M2 12L12 17L22 12" stroke="currentColor" stroke-width="1.5"/>
          </svg>
        </div>
        <div class="header-text">
          <h1>ZRSIM节点监控</h1>
          <span class="subtitle">集群监控中心 | 实时性能监控</span>
        </div>
      </div>
      <div class="header-right">
        <div class="user-info" v-if="userStore.userInfo">
          <span class="user-name">{{ userStore.userInfo.username }}</span>
          <span class="user-role" :class="roleClass">{{ userStore.userInfo.roleName }}</span>
        </div>
        <div class="stats-badge">
          <span class="stats-dot" :class="{ 'connected': wsConnected }"></span>
          <span class="stats-text">{{ wsConnected ? '实时监控中' : '连接中...' }}</span>
          <span class="stats-number">{{ nodes.length }}</span>
        </div>
      </div>
    </div>

    <!-- 主布局 -->
    <div class="main-layout">
      <!-- 左侧节点列表 -->
      <div class="left-sidebar glass-panel">
        <div class="sidebar-header">
          <span class="sidebar-title">📊 节点集群</span>
          <span class="sidebar-total">{{ nodes.length }} 个在线节点</span>
        </div>
        <div class="node-list">
          <div
            v-for="node in nodes"
            :key="node.node_id"
            class="node-item"
            :class="{ active: selectedNode?.node_id === node.node_id, [node.status]: true }"
            @click="selectNode(node)"
          >
            <div class="node-icon">
              <div class="icon-bg" :class="node.status">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                  <rect x="4" y="4" width="16" height="16" rx="2" stroke="currentColor" stroke-width="1.5"/>
                  <path d="M8 2V6M16 2V6M4 10H20" stroke="currentColor" stroke-width="1.5"/>
                </svg>
              </div>
            </div>
            <div class="node-info">
              <div class="node-name">{{ node.hostname || node.node_id.slice(0, 12) }}</div>
              <div class="node-ip">{{ node.ip || '未知IP' }}</div>
            </div>
            <div class="node-status-indicator">
              <div class="status-led" :class="node.status"></div>
            </div>
          </div>
          <div v-if="nodes.length === 0" class="empty-list">
            <div class="empty-illustration">
              <svg width="64" height="64" viewBox="0 0 24 24" fill="none">
                <path d="M12 8V12L15 15" stroke="currentColor" stroke-width="1.5"/>
                <circle cx="12" cy="12" r="9" stroke="currentColor" stroke-width="1.5"/>
              </svg>
            </div>
            <p>等待节点连接...</p>
          </div>
        </div>
      </div>

      <!-- 右侧详情区域 -->
      <div class="right-content">
        <div v-if="selectedNode" class="detail-card glass-panel">
          <!-- 节点基本信息 -->
          <div class="detail-header">
            <div class="header-title-group">
              <div class="host-badge">
                <svg width="24" height="24" viewBox="0 0 24 24" fill="none">
                  <path d="M20 12V8H4V12M20 12L4 12M20 12V16M4 12V16" stroke="currentColor" stroke-width="1.5"/>
                  <rect x="2" y="4" width="20" height="16" rx="2" stroke="currentColor" stroke-width="1.5"/>
                </svg>
                <h2>{{ selectedNode.hostname || selectedNode.node_id }}</h2>
              </div>
              <span :class="['status-chip', selectedNode.status]">
                <span class="pulse" v-if="selectedNode.status === 'online'"></span>
                {{ getStatusText(selectedNode.status) }}
              </span>
            </div>
            <div class="timestamp">
              <svg width="14" height="14" viewBox="0 0 24 24" fill="none">
                <circle cx="12" cy="12" r="9" stroke="currentColor" stroke-width="1.5"/>
                <path d="M12 8V12L15 15" stroke="currentColor" stroke-width="1.5"/>
              </svg>
              {{ formatTime(selectedNode.timestamp) }}
            </div>
          </div>
          
          <!-- 硬件信息 -->
          <div class="system-section">
            <div class="section-title">🖥️ 系统信息</div>
            <div class="system-grid">
              <div class="system-item">
                <div class="item-label">CPU型号</div>
                <div class="item-value">{{ selectedNode.hardware?.cpu || '未获取' }}</div>
              </div>
              <div class="system-item">
                <div class="item-label">CPU核心数</div>
                <div class="item-value">{{ selectedNode.hardware?.cores || selectedNode.cpu_cores?.length || 0 }} 核</div>
              </div>
              <div class="system-item">
                <div class="item-label">操作系统</div>
                <div class="item-value">{{ selectedNode.hardware?.os || 'Windows' }}</div>
              </div>
              <div class="system-item">
                <div class="item-label">节点ID</div>
                <div class="item-value code">{{ selectedNode.node_id }}</div>
              </div>
              <div class="system-item">
                <div class="item-label">IP地址</div>
                <div class="item-value">{{ selectedNode.ip }}</div>
              </div>
              <div class="system-item">
                <div class="item-label">最后更新</div>
                <div class="item-value">{{ formatTime(selectedNode.timestamp) }}</div>
              </div>
            </div>
          </div>
          <!-- CPU 使用率图表 -->
          <div class="chart-section">
            <div class="section-title">
              <span>💻 CPU 使用率</span>
              <span class="section-value">{{ formatNumber(selectedNode.cpu_usage) }}%</span>
            </div>
            <div class="progress-bar">
              <div class="progress-fill cpu-fill" :style="{ width: formatNumber(selectedNode.cpu_usage) + '%' }"></div>
            </div>
            <div class="cpu-cores-grid" v-if="selectedNode.cpu_cores?.length">
              <div v-for="(core, idx) in selectedNode.cpu_cores.slice(0, 8)" :key="idx" class="core-item">
                <span class="core-label">核心 {{ idx + 1 }}</span>
                <div class="core-bar">
                  <div class="core-fill" :style="{ width: formatNumber(core) + '%' }"></div>
                </div>
                <span class="core-value">{{ formatNumber(core) }}%</span>
              </div>
            </div>
          </div>

          <!-- 内存使用率 -->
          <div class="chart-section">
            <div class="section-title">
              <span>🧠 内存使用率</span>
              <span class="section-value">{{ formatNumber(selectedNode.memory?.usage) }}%</span>
            </div>
            <div class="progress-bar">
              <div class="progress-fill memory-fill" :style="{ width: formatNumber(selectedNode.memory?.usage) + '%' }"></div>
            </div>
            <div class="memory-stats">
              <span>已用: {{ formatNumber(selectedNode.memory?.used_gb) }} GB</span>
              <span>总计: {{ formatNumber(selectedNode.memory?.total_gb) }} GB</span>
            </div>
          </div>

          <!-- 磁盘信息 -->
          <div class="chart-section" v-if="selectedNode.disks?.length">
            <div class="section-title">
              <span>💾 磁盘使用率</span>
            </div>
            <div v-for="disk in selectedNode.disks" :key="disk.name" class="disk-item">
              <div class="disk-header">
                <span class="disk-name">{{ disk.name }}</span>
                <span class="disk-usage">{{ formatNumber(disk.usage) }}%</span>
              </div>
              <div class="progress-bar">
                <div class="progress-fill disk-fill" :style="{ width: formatNumber(disk.usage) + '%' }"></div>
              </div>
              <div class="disk-stats">
                <span>已用: {{ formatNumber(Number(disk.size_gb || 0) - Number(disk.free_gb || 0)) }} GB</span>
                <span>可用: {{ formatNumber(disk.free_gb) }} GB</span>
                <span>总容量: {{ formatNumber(disk.size_gb) }} GB</span>
              </div>
            </div>
          </div>

          <!-- GPU 信息 -->
          <div class="chart-section" v-if="selectedNode.gpus?.length">
            <div class="section-title">
              <span>🎮 GPU 信息</span>
            </div>
            <div v-for="(gpu, idx) in selectedNode.gpus" :key="idx" class="gpu-item">
              <div class="gpu-header">
                <span class="gpu-name">{{ gpu.model || gpu.name || '未知GPU' }}</span>
              </div>
            </div>
          </div>

          <!-- 进程信息 -->
          <div class="chart-section" v-if="selectedNode.processes?.length">
            <div class="section-title">
              <span>📈 进程监控 (Top 10)</span>
            </div>
            <div class="process-list">
              <div v-for="(proc, idx) in selectedNode.processes.slice(0, 10)" :key="idx" class="process-item">
                <span class="process-name" :title="proc.name">{{ proc.name || '未知进程' }}</span>
                <div class="process-memory-bar">
                  <div class="process-memory-fill" :style="{ width: getProcessWidth(proc.memory_mb, selectedNode.memory?.total_gb) + '%' }"></div>
                </div>
                <span class="process-memory">{{ formatNumber(proc.memory_mb) }} MB</span>
              </div>
            </div>
          </div>

        </div>

        <!-- 未选择节点提示 -->
        <div v-else class="empty-detail glass-panel">
          <div class="empty-animation">
            <div class="pulse-ring"></div>
            <div class="empty-icon">
              <svg width="80" height="80" viewBox="0 0 24 24" fill="none">
                <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.2"/>
                <path d="M2 17L12 22L22 17" stroke="currentColor" stroke-width="1.2"/>
                <path d="M2 12L12 17L22 12" stroke="currentColor" stroke-width="1.2"/>
                <circle cx="12" cy="12" r="2" fill="currentColor" fill-opacity="0.6"/>
              </svg>
            </div>
          </div>
          <p>选择节点查看详细监控数据</p>
          <span class="empty-hint">点击左侧节点卡片查看 CPU、内存、磁盘、进程等实时信息</span>
        </div>
      </div>
    </div>

    <!-- 调试面板 -->
    <div class="debug-console" v-if="showDebug">
      <div class="debug-header">
        <span>🔍 WebSocket 调试信息</span>
        <button @click="showDebug = false">关闭</button>
      </div>
      <div class="debug-content">
        <div>✅ 连接状态: {{ wsConnected ? '已连接' : '未连接' }}</div>
        <div>📊 节点数量: {{ nodes.length }}</div>
        <div>📨 最后消息: {{ lastMessage || '无' }}</div>
      </div>
    </div>
    <div class="debug-toggle" @click="showDebug = !showDebug">
      🐛
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted,computed } from 'vue'
import { useRouter } from 'vue-router'  // 添加导入
import { useUserStore } from '../stores/user'  // 添加导入
const router = useRouter()
const userStore = useUserStore()

// 添加角色样式计算属性
const roleClass = computed(() => {
  if (!userStore.userInfo) return ''
  switch (userStore.userInfo.role) {
    case 0: return 'super-admin'
    case 1: return 'admin'
    case 2: return 'designer'
    default: return 'viewer'
  }
})

// 添加返回主页函数
const goBack = () => {
  router.push('/')
}

interface NodeInfo {
  node_id: string
  ip: string
  hostname?: string
  hardware: {
    cpu: string
    cores: number
    memory: string
    disk: string
    os: string
  }
  timestamp: number
  status: 'online' | 'offline' | 'busy'
  cpu_usage?: number | string
  cpu_cores?: (number | string)[]
  memory?: {
    total_gb: number | string
    used_gb: number | string
    usage: number | string
  }
  disks?: Array<{
    name: string
    size_gb: number | string
    free_gb: number | string
    usage: number | string
  }>
  gpus?: Array<{
    name: string
    model: string
  }>
  processes?: Array<{
    name: string
    memory_mb: number | string
  }>
}

const nodes = ref<NodeInfo[]>([])
const selectedNode = ref<NodeInfo | null>(null)
const wsConnected = ref(false)
const lastMessage = ref('')
const showDebug = ref(false)

let ws: WebSocket | null = null
let reconnectTimer: number | null = null
let heartbeatTimer: number | null = null

// 安全地格式化数字
const formatNumber = (value: any, decimals: number = 1): number => {
  if (value === undefined || value === null) return 0
  const num = parseFloat(value)
  return isNaN(num) ? 0 : parseFloat(num.toFixed(decimals))
}

const getStatusText = (status: string) => {
  const map: Record<string, string> = { online: '在线', offline: '离线', busy: '繁忙' }
  return map[status] || status
}

const formatTime = (timestamp: number) => {
  if (!timestamp) return '未知'
  return new Date(timestamp * 1000).toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  })
}

const getProcessWidth = (memoryMB: any, totalGB: any = 16): number => {
  const mb = formatNumber(memoryMB)
  const totalMB = formatNumber(totalGB) * 1024
  if (totalMB === 0) return 0
  return Math.min((mb / totalMB) * 100, 100)
}

const selectNode = (node: NodeInfo) => {
  selectedNode.value = node
}

const processNodeData = (data: any) => {
  if (!data.node_id) return

  // 安全地处理数值数据
  const safeNumber = (val: any, defaultVal: number = 0): number => {
    const num = parseFloat(val)
    return isNaN(num) ? defaultVal : num
  }

  const nodeData: NodeInfo = {
    node_id: data.node_id,
    ip: data.ip || '未知IP',
    hostname: data.hostname || data.node_id,
    hardware: {
      cpu: data.hardware?.cpu || '未获取',
      cores: data.hardware?.cores || data.cpu_cores?.length || 0,
      memory: data.hardware?.memory || `${safeNumber(data.memory?.total_gb, 16)}GB`,
      disk: data.hardware?.disk || 'System Drive',
      os: data.hardware?.os || 'Windows 11'
    },
    timestamp: data.timestamp || Date.now(),
    status: data.status || 'online',
    cpu_usage: safeNumber(data.cpu_usage),
    cpu_cores: Array.isArray(data.cpu_cores) ? data.cpu_cores.map((c: any) => safeNumber(c)) : [],
    memory: {
      total_gb: safeNumber(data.memory?.total_gb, 16),
      used_gb: safeNumber(data.memory?.used_gb, 8),
      usage: safeNumber(data.memory?.usage, 50)
    },
    disks: Array.isArray(data.disks) ? data.disks.map((disk: any) => ({
      name: disk.name || '未知',
      size_gb: safeNumber(disk.size_gb),
      free_gb: safeNumber(disk.free_gb),
      usage: safeNumber(disk.usage)
    })) : [],
    gpus: Array.isArray(data.gpus) ? data.gpus.map((gpu: any) => ({
      name: gpu.name || 'GPU',
      model: gpu.model || gpu.name || '未知'
    })) : [],
    processes: Array.isArray(data.processes) ? data.processes.map((proc: any) => ({
      name: proc.name || '未知进程',
      memory_mb: safeNumber(proc.memory_mb)
    })) : []
  }

  const index = nodes.value.findIndex(item => item.node_id === nodeData.node_id)
  if (index >= 0) {
    nodes.value[index] = { ...nodeData }
    if (selectedNode.value?.node_id === nodeData.node_id) {
      selectedNode.value = { ...nodeData }
    }
  } else {
    nodes.value.push({ ...nodeData })
    if (nodes.value.length === 1) {
      selectedNode.value = nodeData
    }
  }
}

const connectWebSocket = () => {
//  const wsUrl = 'ws://212.129.221.186:8088'
 const wsUrl = 'ws://192.168.156.20:8088'
  //const wsUrl = 'ws://localhost:8088'
  ws = new WebSocket(wsUrl)

  ws.onopen = () => {
    console.log('WebSocket连接成功')
    wsConnected.value = true
    startHeartbeat()
  }

  ws.onmessage = (event) => {
    const messageData = event.data
    lastMessage.value = typeof messageData === 'string' ? messageData.substring(0, 200) : '二进制数据'

    try {
      if (typeof messageData === 'string') {
        const parsedData = JSON.parse(messageData)
        processNodeData(parsedData)
      }
    } catch (err) {
      console.error('JSON解析失败:', err)
    }
  }

  ws.onerror = (error) => {
    console.error('WebSocket错误:', error)
    wsConnected.value = false
  }

  ws.onclose = () => {
    console.log('WebSocket连接关闭')
    wsConnected.value = false
    stopHeartbeat()
    reconnectWebSocket()
  }
}

const startHeartbeat = () => {
  heartbeatTimer = window.setInterval(() => {
    if (ws?.readyState === WebSocket.OPEN) {
      ws.send('heartbeat')
    }
  }, 30000)
}

const stopHeartbeat = () => {
  if (heartbeatTimer) {
    clearInterval(heartbeatTimer)
    heartbeatTimer = null
  }
}

const reconnectWebSocket = () => {
  if (reconnectTimer) return
  reconnectTimer = window.setTimeout(() => {
    connectWebSocket()
    reconnectTimer = null
  }, 3000)
}

onMounted(() => {
  connectWebSocket()
})

onUnmounted(() => {
  stopHeartbeat()
  if (reconnectTimer) clearTimeout(reconnectTimer)
  if (ws) {
    ws.close()
    ws = null
  }
})
</script>

<style scoped>
/* 样式保持不变，与之前相同 */
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

html, body {
  width: 100%;
  height: 100%;
  overflow: hidden;
}

.monitor-container {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  width: 100vw;
  height: 100vh;
  background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
  font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.bg-decoration {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  overflow: hidden;
  z-index: 0;
}

.gradient-sphere {
  position: absolute;
  border-radius: 50%;
  filter: blur(80px);
  opacity: 0.3;
  animation: float 20s ease-in-out infinite;
}

.sphere-1 {
  width: 500px;
  height: 500px;
  background: radial-gradient(circle, #3b82f6, #06b6d4);
  top: -200px;
  right: -100px;
}

.sphere-2 {
  width: 400px;
  height: 400px;
  background: radial-gradient(circle, #8b5cf6, #ec489a);
  bottom: -150px;
  left: -150px;
  animation-delay: -5s;
}

.sphere-3 {
  width: 300px;
  height: 300px;
  background: radial-gradient(circle, #10b981, #06b6d4);
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  animation-delay: -10s;
}

@keyframes float {
  0%, 100% { transform: translate(0, 0) scale(1); }
  50% { transform: translate(30px, 20px) scale(1.1); }
}

.glass-panel {
  background: rgba(15, 25, 35, 0.7);
  backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 28px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
}

.header {
  position: relative;
  z-index: 10;
  margin: 20px 24px 0 24px;
  padding: 16px 28px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-shrink: 0;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 16px;
}

.logo-icon {
  width: 44px;
  height: 44px;
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
}

.header-text h1 {
  font-size: 24px;
  font-weight: 600;
  background: linear-gradient(135deg, #fff, #94a3b8);
  -webkit-background-clip: text;
  background-clip: text;
  color: transparent;
  margin: 0;
}

.subtitle {
  font-size: 12px;
  color: #94a3b8;
  margin-top: 2px;
  display: block;
}

.header-right .stats-badge {
  display: flex;
  align-items: center;
  gap: 12px;
  background: rgba(59, 130, 246, 0.15);
  padding: 8px 20px;
  border-radius: 40px;
  border: 1px solid rgba(59, 130, 246, 0.3);
}

.stats-dot {
  width: 8px;
  height: 8px;
  background: #64748b;
  border-radius: 50%;
  transition: all 0.3s;
}

.stats-dot.connected {
  background: #10b981;
  box-shadow: 0 0 8px #10b981;
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0%, 100% { opacity: 1; transform: scale(1); }
  50% { opacity: 0.6; transform: scale(1.2); }
}

.stats-text {
  color: #cbd5e1;
  font-size: 14px;
}

.stats-number {
  color: white;
  font-weight: 700;
  font-size: 20px;
  margin-left: 4px;
}

.main-layout {
  display: flex;
  gap: 24px;
  padding: 24px;
  position: relative;
  z-index: 10;
  flex: 1;
  min-height: 0;
  overflow: hidden;
}

.left-sidebar {
  width: 320px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  flex-shrink: 0;
}

.sidebar-header {
  padding: 20px 24px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  flex-shrink: 0;
}

.sidebar-title {
  font-size: 16px;
  font-weight: 600;
  color: white;
}

.sidebar-total {
  font-size: 13px;
  color: #64748b;
  background: rgba(0, 0, 0, 0.3);
  padding: 4px 10px;
  border-radius: 20px;
}

.node-list {
  flex: 1;
  overflow-y: auto;
  padding: 12px;
}

.node-list::-webkit-scrollbar {
  width: 4px;
}

.node-list::-webkit-scrollbar-track {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

.node-list::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

.node-item {
  display: flex;
  align-items: center;
  gap: 14px;
  padding: 14px 16px;
  margin-bottom: 8px;
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.25s ease;
  background: rgba(255, 255, 255, 0.02);
  border: 1px solid transparent;
}

.node-item:hover {
  background: rgba(59, 130, 246, 0.1);
  border-color: rgba(59, 130, 246, 0.3);
  transform: translateX(4px);
}

.node-item.active {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.2), rgba(139, 92, 246, 0.1));
  border-color: rgba(59, 130, 246, 0.5);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
}

.icon-bg {
  width: 40px;
  height: 40px;
  border-radius: 14px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.node-info {
  flex: 1;
}

.node-name {
  font-weight: 600;
  color: white;
  font-size: 15px;
  margin-bottom: 4px;
}

.node-ip {
  font-size: 11px;
  color: #94a3b8;
  font-family: monospace;
}

.node-status-indicator {
  display: flex;
  align-items: center;
}

.status-led {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #10b981;
  box-shadow: 0 0 6px #10b981;
}

.status-led.offline {
  background: #ef4444;
  box-shadow: 0 0 6px #ef4444;
}

.empty-list {
  text-align: center;
  padding: 60px 20px;
  color: #64748b;
}

.empty-illustration svg {
  color: #334155;
  margin-bottom: 16px;
}

.right-content {
  flex: 1;
  min-width: 0;
  overflow-y: auto;
  overflow-x: hidden;
}

.right-content::-webkit-scrollbar {
  width: 6px;
}

.right-content::-webkit-scrollbar-track {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

.right-content::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

.detail-card {
  width: 100%;
  padding: 28px 32px;
  animation: fadeInUp 0.4s ease;
  margin-bottom: 24px;
}

@keyframes fadeInUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.detail-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 28px;
  flex-wrap: wrap;
  gap: 16px;
}

.header-title-group {
  display: flex;
  align-items: center;
  gap: 16px;
  flex-wrap: wrap;
}

.host-badge {
  display: flex;
  align-items: center;
  gap: 10px;
  background: rgba(255, 255, 255, 0.05);
  padding: 8px 20px;
  border-radius: 40px;
}

.host-badge svg {
  color: #3b82f6;
}

.host-badge h2 {
  font-size: 20px;
  font-weight: 600;
  color: white;
  margin: 0;
}

.status-chip {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  padding: 6px 16px;
  border-radius: 40px;
  font-size: 13px;
  font-weight: 500;
  background: rgba(0, 0, 0, 0.3);
}

.status-chip.online {
  color: #10b981;
  border: 1px solid rgba(16, 185, 129, 0.3);
}

.timestamp {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 12px;
  color: #64748b;
  background: rgba(0, 0, 0, 0.3);
  padding: 6px 12px;
  border-radius: 20px;
}

.chart-section {
  margin-bottom: 28px;
  padding: 20px;
  background: rgba(0, 0, 0, 0.2);
  border-radius: 24px;
}

.section-title {
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-size: 14px;
  font-weight: 600;
  color: #94a3b8;
  margin-bottom: 12px;
}

.section-value {
  font-size: 20px;
  font-weight: 700;
  color: #3b82f6;
}

.progress-bar {
  height: 8px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 10px;
  overflow: hidden;
  margin: 12px 0;
}

.progress-fill {
  height: 100%;
  border-radius: 10px;
  transition: width 0.3s ease;
}

.cpu-fill {
  background: linear-gradient(90deg, #3b82f6, #8b5cf6);
}

.memory-fill {
  background: linear-gradient(90deg, #10b981, #06b6d4);
}

.disk-fill {
  background: linear-gradient(90deg, #f59e0b, #ef4444);
}

.cpu-cores-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
  margin-top: 16px;
}

.core-item {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.core-label {
  font-size: 11px;
  color: #94a3b8;
}

.core-bar {
  height: 4px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 4px;
  overflow: hidden;
}

.core-fill {
  height: 100%;
  background: #3b82f6;
  border-radius: 4px;
  transition: width 0.3s ease;
}

.core-value {
  font-size: 10px;
  color: #cbd5e1;
}

.memory-stats, .disk-stats {
  display: flex;
  gap: 16px;
  font-size: 12px;
  color: #94a3b8;
  margin-top: 8px;
}

.disk-item {
  margin-bottom: 20px;
}

.disk-header {
  display: flex;
  justify-content: space-between;
  margin-bottom: 6px;
}

.disk-name {
  font-weight: 500;
  color: white;
}

.disk-usage {
  font-size: 12px;
  color: #f59e0b;
}

.gpu-item {
  padding: 12px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
  margin-bottom: 8px;
}

.gpu-name {
  color: white;
  font-size: 13px;
}

.process-list {
  max-height: 300px;
  overflow-y: auto;
}

.process-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 8px 0;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
}

.process-name {
  flex: 1;
  font-size: 13px;
  color: #cbd5e1;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.process-memory-bar {
  width: 150px;
  height: 4px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 4px;
  overflow: hidden;
}

.process-memory-fill {
  height: 100%;
  background: linear-gradient(90deg, #8b5cf6, #ec489a);
  border-radius: 4px;
}

.process-memory {
  width: 70px;
  font-size: 11px;
  color: #94a3b8;
  text-align: right;
}

.system-section {
  background: rgba(0, 0, 0, 0.2);
  border-radius: 24px;
  padding: 20px;
  margin-top: 8px;
  margin-bottom: 28px;
}

.system-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 16px;
  margin-top: 12px;
}

.system-item {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.item-label {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 11px;
  color: #94a3b8;
  font-weight: 500;
}

.item-value {
  font-size: 13px;
  color: #e2e8f0;
  word-break: break-all;
}

.item-value.code {
  font-family: monospace;
  font-size: 11px;
  background: rgba(0, 0, 0, 0.4);
  padding: 4px 8px;
  border-radius: 8px;
  display: inline-block;
}

.empty-detail {
  width: 100%;
  max-width: 500px;
  margin: 0 auto;
  text-align: center;
  padding: 60px 40px;
  position: relative;
  top: 50%;
  transform: translateY(-50%);
}

.empty-animation {
  position: relative;
  width: 120px;
  height: 120px;
  margin: 0 auto 24px;
}

.pulse-ring {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 100px;
  height: 100px;
  border-radius: 50%;
  border: 2px solid rgba(59, 130, 246, 0.3);
  animation: ringPulse 2s infinite;
}

@keyframes ringPulse {
  0% {
    width: 80px;
    height: 80px;
    opacity: 0.6;
  }
  100% {
    width: 140px;
    height: 140px;
    opacity: 0;
  }
}

.empty-icon {
  position: relative;
  z-index: 2;
  color: #3b82f6;
}

.empty-detail p {
  font-size: 18px;
  font-weight: 500;
  color: white;
  margin-bottom: 8px;
}

.empty-hint {
  font-size: 13px;
  color: #64748b;
}

.debug-console {
  position: fixed;
  bottom: 20px;
  right: 20px;
  width: 400px;
  max-height: 400px;
  background: rgba(0, 0, 0, 0.95);
  backdrop-filter: blur(10px);
  border-radius: 16px;
  border: 1px solid rgba(59, 130, 246, 0.3);
  z-index: 1000;
  font-size: 12px;
  font-family: monospace;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.debug-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 16px;
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
  font-weight: 500;
}

.debug-header button {
  background: rgba(255, 255, 255, 0.1);
  border: none;
  color: white;
  padding: 4px 12px;
  border-radius: 8px;
  cursor: pointer;
  font-size: 11px;
}

.debug-content {
  padding: 12px 16px;
  color: #94a3b8;
  overflow-y: auto;
  max-height: 350px;
}

.debug-toggle {
  position: fixed;
  bottom: 20px;
  right: 20px;
  width: 40px;
  height: 40px;
  background: rgba(59, 130, 246, 0.8);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  z-index: 1001;
  font-size: 20px;
  box-shadow: 0 2px 10px rgba(0, 0, 0, 0.3);
  transition: all 0.2s;
}

.debug-toggle:hover {
  transform: scale(1.1);
}

@media (max-width: 1024px) {
  .cpu-cores-grid {
    grid-template-columns: repeat(2, 1fr);
  }
  
  .system-grid {
    grid-template-columns: 1fr;
  }
  
  .left-sidebar {
    width: 280px;
  }
  
  .debug-console {
    width: 350px;
  }
}

@media (max-width: 768px) {
  .main-layout {
    flex-direction: column;
  }
  
  .left-sidebar {
    width: 100%;
    max-height: 300px;
  }
  
  .detail-card {
    padding: 20px;
  }
  
  .cpu-cores-grid {
    grid-template-columns: 1fr;
  }
  
  .debug-console {
    width: calc(100% - 40px);
    right: 20px;
    bottom: 70px;
  }
  
  .empty-detail {
    top: 0;
    transform: none;
    margin-top: 40px;
  }
  .back-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 36px;
  height: 36px;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.2s;
  color: #94a3b8;
}

.back-icon:hover {
  background: rgba(255, 255, 255, 0.1);
  color: white;
}

.header-left {
  display: flex;
  align-items: center;
  gap: 12px;
}
}
</style>