<template>
  <BaseEdge :path="path" :labelX="labelX" :labelY="labelY" :label="label" :style="edgeStyle" />
</template>

<script setup lang="ts">
import { BaseEdge, getBezierPath, getSmoothStepPath } from '@vue-flow/core'

const props = defineProps<{
  id: string
  sourceX: number
  sourceY: number
  targetX: number
  targetY: number
  sourcePosition: any
  targetPosition: any
  label?: string
  data?: any
}>()

import { computed } from 'vue'

const bezier = computed(() => getBezierPath({
  sourceX: props.sourceX,
  sourceY: props.sourceY,
  targetX: props.targetX,
  targetY: props.targetY,
  sourcePosition: props.sourcePosition,
  targetPosition: props.targetPosition,
}))

const path = computed(() => bezier.value[0])
const labelX = computed(() => bezier.value[1])
const labelY = computed(() => bezier.value[2])

const edgeStyle = {
  stroke: '#409eff',
  strokeWidth: 1,
}
</script>