// components/CustomNode.vue
<template>
  <div class="custom-node">
    <div class="node-header">
      <span>{{ data.label }}</span>
    </div>
    <div class="node-content" :style="{ minHeight: minHeight }">
      <div class="handles-left">
        <div
          v-for="(input, idx) in inputs"
          :key="`in-${idx}`"
          class="handle-item left-item"
          :style="{ top: `${((idx + 1) / (inputs.length + 1)) * 100}%` }"
        >
          <Handle
            type="target"
            :position="Position.Left"
            :id="`input-${input.name}`"
            class="handle-port"
          />
          <span class="handle-label">{{ input.name }}</span>
        </div>
      </div>
      <div class="handles-right">
        <div
          v-for="(output, idx) in outputs"
          :key="`out-${idx}`"
          class="handle-item right-item"
          :style="{ top: `${((idx + 1) / (outputs.length + 1)) * 100}%` }"
        >
          <span class="handle-label">{{ output.name }}</span>
          <Handle
            type="source"
            :position="Position.Right"
            :id="`output-${output.name}`"
            class="handle-port"
          />
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { Handle, Position ,type NodeProps  } from '@vue-flow/core'
import type { ModelInput, ModelOutput, ParsedModel } from '@/utils/xmlParser'

type CustomNodeData = {
    model: ParsedModel
    inputs: ModelInput[]
    outputs: ModelOutput[]
    label: string
}

const props = defineProps<NodeProps<CustomNodeData>>()
const inputs = props.data.inputs
const outputs = props.data.outputs
const minHeight = computed(() => {
  const count = Math.max(inputs?.length ?? 0, outputs?.length ?? 0)
  const rowHeight = 34
  const padding = 24
  return `${Math.max(80, count * rowHeight + padding)}px`
})
</script>

<style scoped>
.custom-node {
  background: white;
  border-radius: 8px;
  border: 1px solid #409eff;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  min-width: 160px;
  background-color: #fff;
}
.node-header {
  background: #409eff;
  color: white;
  padding: 6px 12px;
  border-radius: 8px 8px 0 0;
  font-weight: bold;
  text-align: center;
}
.node-content {
  position: relative;
  padding: 12px 8px 16px;
}
.handles-left,
.handles-right {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
}
.handle-item {
  position: absolute;
  width: 100%;
  display: flex;
  align-items: center;
  gap: 5px;
  transform: translateY(-50%);
}
.left-item {
  justify-content: flex-start;
  padding-left: 6px;
}
.right-item {
  justify-content: flex-end;
  padding-right: 6px;
}
.handle-label {
  font-size: 12px;
  color: #333;
  background: #f0f0f0;
  padding: 2px 2px;
  border-radius: 2px;
  margin: 0px;
}

.handle-port {
    background-color: '#4CAF50';
    width: 6px;
    height: 6px;
    border-radius: px;
    border: 1px solid #2c3e50;
}

.handle-port:hover {
    border: 1px solid #409eff;
}

</style>