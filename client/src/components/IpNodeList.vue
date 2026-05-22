// components/IpNodeList.vue
<template>
  <div class="ip-list">
    <h4>设备信息</h4>
    <el-table :data="ipNodes" size="small" height="calc(100% - 40px)">
      <el-table-column prop="ip" label="IP地址" />
      <el-table-column prop="status" label="状态">
        <template #default="{ row }">
          <el-tag :type="row.status === 'online' ? 'success' : 'danger'" size="small">
            {{ row.status === 'online' ? '在线' : '离线' }}
          </el-tag>
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { ElTable, ElTableColumn, ElTag } from 'element-plus'

interface IpNode {
  ip: string
  status: 'online' | 'offline'
}

const ipNodes = ref<IpNode[]>([])
let wsInterval: ReturnType<typeof setInterval> | null = null

// 模拟WebSocket获取在线IP
const fetchOnlineNodes = () => {
  const mockIps = [
    '192.168.1.101', '192.168.1.102', '192.168.1.103',
    '10.0.0.25', '10.0.0.36', '172.16.0.5'
  ]
  const randomOnline = mockIps.slice(0, Math.floor(Math.random() * 4) + 2)
  ipNodes.value = mockIps.map(ip => ({
    ip,
    status: randomOnline.includes(ip) ? 'online' : 'offline'
  }))
}

onMounted(() => {
  fetchOnlineNodes()
  wsInterval = setInterval(fetchOnlineNodes, 5000)
})

onUnmounted(() => {
  if (wsInterval) clearInterval(wsInterval)
})
</script>

<style scoped>
.ip-list {
  width: 100%;
  height: 100%;
  padding: 12px;
  box-sizing: border-box;
  background: #ffffff;
  border-radius: 12px;
  border: 1px solid rgba(0, 0, 0, 0.06);
  box-shadow: 0 1px 8px rgba(0, 0, 0, 0.06);
  overflow: hidden;
}
.ip-list h4 {
  margin: 0 0 12px 0;
  font-size: 14px;
  color: #303133;
}
</style>