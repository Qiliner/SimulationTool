<template>
  <div class="directory-node">
    <div 
      class="directory-item"
      :style="{ paddingLeft: (level * 20 + 8) + 'px' }"
      @click="$emit('select', directory.id)"
    >
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <path d="M3 7v10a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2h-6l-2-2H5a2 2 0 0 0-2 2z"/>
      </svg>
      <span class="dir-name">{{ directory.name }}</span>
      <div class="dir-actions">
        <button class="dir-btn" @click.stop="$emit('rename', directory)" title="重命名">✎</button>
        <button class="dir-btn" @click.stop="$emit('delete', directory)" title="删除">×</button>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
defineProps<{
  directory: any
  level: number
}>()

defineEmits(['select', 'delete', 'rename'])
</script>

<style scoped>
.directory-node {
  margin-bottom: 4px;
}

.directory-item {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 6px 12px;
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s;
  color: #94a3b8;
}

.directory-item:hover {
  background: rgba(59, 130, 246, 0.1);
  color: white;
}

.directory-item svg {
  flex-shrink: 0;
}

.dir-name {
  flex: 1;
  font-size: 13px;
}

.dir-actions {
  display: flex;
  gap: 4px;
  opacity: 0;
  transition: opacity 0.2s;
}

.directory-item:hover .dir-actions {
  opacity: 1;
}

.dir-btn {
  width: 20px;
  height: 20px;
  border-radius: 4px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: transparent;
  border: none;
  color: #94a3b8;
  cursor: pointer;
  font-size: 12px;
}

.dir-btn:hover {
  background: rgba(255, 255, 255, 0.1);
  color: white;
}
</style>