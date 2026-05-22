<template>
    <div class="monitor-panel">
        <div class="monitor-header">
            日志窗口
            <el-button size="small" @click="logStore.clearLogs">清空</el-button>
        </div>
        <div class="monitor-content" id="log-window">
            <div
            v-for="(log, index) in logStore.logs"
            :key="index"
            :class="['monitor-line', log.type]"
            >
            <span class="monitor-timestamp">[{{ log.timestamp }}]</span>
            {{ log.message }}
            </div>
        </div>
        </div>
</template>

<script setup lang="ts">
import { useLogStore } from '@/stores/log'
import { ElButton } from 'element-plus'

const logStore = useLogStore()

</script>


<style scoped>

.monitor-panel {
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  height: 100%;
}
.monitor-header {
  background: linear-gradient(135deg, rgba(142, 45, 226, 0.2), rgba(74, 0, 224, 0.2));
  padding: 10px 16px;
  color: white;
  font-weight: 600;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}
.monitor-content {
  flex: 1;
  overflow-y: auto;
  padding: 12px;
  font-family: 'Consolas', monospace;
  font-size: 12px;
  background: rgba(0, 0, 0, 0.2);
  color: #cbd5e1;
}
.monitor-line {
  padding: 4px 0;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
}
.monitor-line.info { color: #60a5fa; }
.monitor-line.error { color: #f87171; }
.monitor-line.warning { color: #fbbf24; }
.monitor-timestamp {
  color: #6a9955;
  margin-right: 12px;
}

</style>

