<template>
  <div class="tree-node-item">
    <!-- 基础类型 / 结构体（非sequence/非fixedArray） -->
    <div v-if="fieldMeta.kind !== 'sequence' && fieldMeta.kind !== 'fixedArray'" class="node-content">
      <div class="field-label">
        <span class="field-name">{{ label || '字段' }}</span>
        <span class="field-type">{{ fieldMeta.typeName }}</span>
        <span class="field-note" v-if="note">({{ note }})</span>
      </div>
      <div class="field-value">
        <template v-if="fieldMeta.kind === 'basic'">
          <input v-if="fieldMeta.typeName !== 'bool'" type="text" :value="localValue" @input="onInput" />
          <input v-else type="checkbox" class="bool-checkbox" :checked="localValue" @change="onCheckboxChange" />
        </template>
        <div v-else-if="fieldMeta.kind === 'struct'" class="object-children">
          <PropertyEditor
            v-for="subField in fieldMeta.fields"
            :key="getChildPath(subField.name)"
            :field-meta="subField"
            :value="localValue ? localValue[subField.name] : undefined"
            :path="getChildPath(subField.name)"
            :label="subField.displayName"
            :note="subField.note"
            @update="handleChildUpdate"
          />
        </div>
      </div>
    </div>

    <!-- 固定长度数组 -->
    <div v-if="fieldMeta.kind === 'fixedArray'">
      <div class="field-label">
        <span class="field-name">{{ label || '数组' }}</span>
        <span class="field-type">固定数组[{{ fieldMeta.arraySize }}]</span>
      </div>
      <div class="array-items">
        <div v-for="(item, idx) in localValue" :key="idx" class="array-item">
          <div class="array-header">
            <span style="font-size:0.8rem;">索引 {{ idx }}</span>
          </div>
          <PropertyEditor
            :field-meta="fieldMeta.elementMeta"
            :value="item"
            :path="getArrayItemPath(idx)"
            @update="handleChildUpdate"
          />
        </div>
      </div>
    </div>

    <!-- 动态 sequence -->
    <div v-if="fieldMeta.kind === 'sequence'">
      <div class="field-label">
        <span class="field-name">{{ label || '动态数组' }}</span>
        <span class="field-type">sequence (可增删)</span>
      </div>
      <div class="array-controls">
        <button class="btn-icon" @click="addItem">➕ 添加元素</button>
      </div>
      <div v-if="localValue && localValue.length === 0" class="empty-array">暂无数据，点击添加</div>
      <div v-for="(item, idx) in localValue" :key="idx" class="array-item">
        <div class="array-header">
          <span style="font-size:0.8rem;">#{{ idx + 1 }}</span>
          <button class="btn-icon danger" @click="removeItem(idx)">🗑️ 删除</button>
        </div>
        <PropertyEditor
          :field-meta="fieldMeta.elementMeta"
          :value="item"
          :path="getArrayItemPath(idx)"
          @update="handleChildUpdate"
        />
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { computed } from 'vue';
import type { FieldMeta } from '@/utils/types';
import { getDefaultValue } from '../utils/xmlParser';

const props = defineProps<{
  fieldMeta: FieldMeta;
  value: any;
  path: string;
  label?: string;
  note?: string;
}>();

const emit = defineEmits<{
  (e: 'update', payload: { path: string; value: any }): void;
}>();

const localValue = computed({
  get: () => props.value,
  set: (newVal) => {
    emit('update', { path: props.path, value: newVal });
  },
});

const onInput = (e: Event) => {
  const target = e.target as HTMLInputElement;
  let val: any = target.value;
  const typeName = (props.fieldMeta as any).typeName;
  if (typeName === 'int32' || typeName === 'int64' || typeName === 'uint32') {
    val = parseInt(val, 10);
    if (isNaN(val)) val = 0;
  } else if (typeName === 'real32' || typeName === 'real64') {
    val = parseFloat(val);
    if (isNaN(val)) val = 0.0;
  }
  localValue.value = val;
};

const onCheckboxChange = (e: Event) => {
  const target = e.target as HTMLInputElement;
  localValue.value = target.checked;
};

const handleChildUpdate = (event: { path: string; value: any }) => {
  // 拼接子路径，注意处理数组索引标记
  const newPath = `${event.path}`;
  //const newPath = `${props.path}.${event.path}`;
  emit('update', { path: newPath, value: event.value });
};

const getChildPath = (childName: string) => {
  return `${props.path}.${childName}`;
};

const getArrayItemPath = (index: number) => {
  return `${props.path}[${index}]`;
};

const addItem = () => {
  const current = Array.isArray(localValue.value) ? localValue.value : [];
  const elementMeta = (props.fieldMeta as any).elementMeta;
  if (!elementMeta) {
    console.error('sequence elementMeta 缺失', props.fieldMeta);
    return;
  }
  const newItem = getDefaultValue(elementMeta);
  const newArr = [...current, newItem];
  // 直接触发更新，路径为当前 sequence 字段的完整路径
  emit('update', { path: props.path, value: newArr });
};

const removeItem = (index: number) => {
  const current = [...(localValue.value || [])];
  current.splice(index, 1);
  emit('update', { path: props.path, value: current });
};
</script>

<style scoped>
/* 样式基本同原内联，此处略，实际可复制原有样式 */
.tree-node-item { margin: 6px 0; border-left: 2px solid transparent; }
.tree-node-item:hover { border-left-color: #3498db; background: #f8fafc; }
.node-content { display: flex; align-items: flex-start; gap: 12px; padding: 8px 12px; flex-wrap: wrap; }
.field-label { min-width: 180px; font-weight: 500; display: flex; gap: 8px; align-items: baseline; }
.field-name { font-weight: 600; }
.field-type { font-size: 0.7rem; background: #e9ecef; padding: 2px 8px; border-radius: 20px; }
.field-note { font-size: 0.7rem; color: #7f8c8d; }
.field-value { flex: 1; min-width: 200px; }
input, select { padding: 6px 12px; border: 1px solid #dcdfe6; border-radius: 6px; width: 100%; max-width: 280px; }
.bool-checkbox { width: 20px; height: 20px; }
.array-controls { display: flex; gap: 12px; margin: 8px 0 12px; }
.btn-icon { background: #eef2f6; border: none; padding: 4px 12px; border-radius: 30px; cursor: pointer; font-size: 0.75rem; }
.btn-icon.danger { background: #fee2e2; color: #c0392b; }
.array-item { margin-left: 28px; border-left: 1px dashed #cbd5e1; padding-left: 16px; margin-bottom: 12px; }
.empty-array { color: #94a3b8; font-style: italic; padding: 8px 0; }
.object-children { margin-left: 24px; border-left: 1px solid #e2e8f0; padding-left: 16px; }
</style>