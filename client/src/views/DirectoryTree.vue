<template>
  <div class="directory-tree-node" v-if="directory">
    <div 
      class="directory-item"
      :class="{ active: isActive }"
      :style="{ paddingLeft: (level * 20 + 8) + 'px' }"
      @click="$emit('select', directory.id)"
    >
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <path d="M3 7v10a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2h-6l-2-2H5a2 2 0 0 0-2 2z"/>
      </svg>
      <span class="dir-name" :title="directory.name">{{ directory.name }}</span>
      <span class="dir-count" :class="{ 'has-count': getFileCount > 0 }">
        {{ getFileCount }}
      </span>
      <div class="dir-actions">
        <button class="dir-btn rename" @click.stop="$emit('rename', directory)" title="重命名">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M17 3l4 4-7 7H10v-4l7-7z"/>
            <path d="M4 20l4-4"/>
          </svg>
        </button>
        <button class="dir-btn delete" @click.stop="$emit('delete', directory)" title="删除">
          <svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M18 6L6 18M6 6l12 12"/>
          </svg>
        </button>
      </div>
    </div>
    
    <!-- 递归渲染子目录 -->
    <DirectoryTree
    v-for="dir in children"
    :key="dir.id"
    :directory="dir"
    :level="level + 1"
    :all-directories="allDirectories"
    :file-count-map="fileCountMap"
    :selected-dir-id="selectedDirId"
    @select="$emit('select', $event)"
    @delete="$emit('delete', $event)"
    @rename="$emit('rename', $event)"
    />
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'

const props = defineProps<{
  directory: any
  level: number
  allDirectories?: any[]
  fileCountMap?: Record<number, number>
  selectedDirId?: number
}>()

// 判断当前目录是否被选中
const isActive = computed(() => {
  return props.selectedDirId === props.directory?.id
})

// 获取当前目录的文件数量
const getFileCount = computed((): number => {
  if (!props.fileCountMap || !props.directory) {
    console.log('getFileCount: missing props', { fileCountMap: !!props.fileCountMap, directory: !!props.directory })
    return 0
  }
  const count = props.fileCountMap[props.directory.id] || 0
  console.log(`getFileCount for directory ${props.directory.id} (${props.directory.name}): ${count}`)
  return count
})

// 计算子目录
const children = computed(() => {
  if (!props.directory || !props.allDirectories) return []
  return props.allDirectories.filter((dir: any) => dir && dir.parentId === props.directory.id)
})

defineEmits(['select', 'delete', 'rename'])
</script>

<script lang="ts">
// 递归组件需要自己注册自己
export default {
  name: 'DirectoryTree'
}
</script>

<style scoped>
.directory-tree-node {
  margin-bottom: 2px;
}

.directory-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 12px;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s ease;
  color: #94a3b8;
  position: relative;
}

.directory-item:hover {
  background: rgba(59, 130, 246, 0.15);
  color: #e2e8f0;
}

.directory-item.active {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.25), rgba(139, 92, 246, 0.15));
  color: white;
  border-left: 2px solid #3b82f6;
}

.directory-item:hover .dir-actions {
  opacity: 1;
}

.directory-item svg {
  flex-shrink: 0;
  width: 16px;
  height: 16px;
}

.dir-name {
  flex: 1;
  font-size: 13px;
  font-weight: 500;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.dir-count {
  font-size: 11px;
  font-weight: 500;
  padding: 2px 6px;
  border-radius: 12px;
  background: rgba(100, 116, 139, 0.3);
  color: #94a3b8;
  min-width: 28px;
  text-align: center;
  transition: all 0.2s ease;
}

.dir-count.has-count {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.directory-item.active .dir-count {
  background: rgba(59, 130, 246, 0.3);
  color: #60a5fa;
}

.dir-actions {
  display: flex;
  gap: 4px;
  opacity: 0;
  transition: opacity 0.2s ease;
}

.dir-btn {
  width: 24px;
  height: 24px;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  cursor: pointer;
  transition: all 0.2s ease;
}

.dir-btn.rename {
  color: #94a3b8;
}

.dir-btn.rename:hover {
  background: rgba(59, 130, 246, 0.3);
  color: #3b82f6;
}

.dir-btn.delete {
  color: #94a3b8;
}

.dir-btn.delete:hover {
  background: rgba(239, 68, 68, 0.3);
  color: #ef4444;
}

.dir-btn svg {
  width: 12px;
  height: 12px;
}
</style>