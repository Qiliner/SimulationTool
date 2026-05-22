// components/EventList.vue
<template>
  <div class="event-list">
    <h4>临时事件</h4>
    <div class="toolbar">
      <el-button type="primary" size="small" @click="addEvent">添加事件</el-button>
      <!-- <el-button type="success" size="small" @click="saveEvents">保存数据</el-button> -->
    </div>

    <el-table
      :data="events"
      size="small"
      border
      style="width: 100%;"
      max-height="calc(100% - 70px)"
      empty-text="暂无事件数据"
    >
      <el-table-column label="事件名" prop="name">
        <template #default="{ row }">
          <el-input v-model="row.name" placeholder="事件名" size="small" />
        </template>
      </el-table-column>

      <el-table-column label="事件内容" prop="content">
        <template #default="{ row }">
          <el-input
            v-model="row.content"
            type="textarea"
            placeholder="事件内容"
            size="small"
            :autosize="{ minRows: 2, maxRows: 4 }"
          />
        </template>
      </el-table-column>

      <el-table-column label="操作">
        <template #default="{ row, $index }">
          <el-button size="small" type="text" class="text-button" @click="sendEvent(row)">发送</el-button>
          <el-button size="small" type="text" class="text-button danger" @click="removeEvent($index)">删除</el-button>
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { ElMessage, ElInput, ElButton, ElTable, ElTableColumn } from 'element-plus'

interface CustomEvent {
  id: string
  name: string
  content: string
}

const STORAGE_KEY = 'simulation_tool_event_list'
const events = ref<CustomEvent[]>([])

const loadEvents = () => {
  const raw = localStorage.getItem(STORAGE_KEY)
  if (raw) {
    try {
      const parsed = JSON.parse(raw)
      if (Array.isArray(parsed)) {
        events.value = parsed.map((item: any) => ({
          id: item.id || Date.now().toString(),
          name: item.name || '',
          content: item.content || '',
        }))
        return
      }
    } catch {
      // ignore parse errors
    }
  }

  events.value = [
    { id: '1', name: 'startMission', content: '{"missionId": 1}' },
    { id: '2', name: 'stopRobot', content: '{}' },
  ]
}

const addEvent = () => {
  const newId = Date.now().toString()
  events.value.push({ id: newId, name: '', content: '' })
}

const removeEvent = (idx: number) => {
  events.value.splice(idx, 1)
}

const sendEvent = (event: CustomEvent) => {
  if (!event.name) {
    ElMessage.warning('请输入事件名')
    return
  }

  console.log('[Event Sent]', event.name, event.content)
  ElMessage.success(`已发送事件: ${event.name}`)
}

const saveEvents = () => {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(events.value))
  ElMessage.success('表格数据已保存')
}

onMounted(loadEvents)
</script>

<style scoped>
.event-list {
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
.event-list h4 {
  margin: 0 0 12px 0;
}
.toolbar {
  display: flex;
  gap: 10px;
  margin-bottom: 12px;
}
.text-button {
  color: #409eff;
  padding: 0 2px;
  margin-right: 2px;
}
.text-button.danger {
  color: #f56c6c;
  margin-right: 0;
}
.el-table {
  width: 100%;
}
</style>