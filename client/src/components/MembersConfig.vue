// components/MembersConfig.vue
<template>
  <div class="members-config">
    <div v-if="!selectedNode" class="empty-tip">请选中画布中的节点</div>
    <div v-else>
      <div v-for="member in membersList" :key="member.name" class="member-item">
        <div class="member-header">
          <span class="member-name">{{ member.name }}</span>
          <span class="member-type">{{ member.typeName }}</span>
          <el-button v-if="member.isSequence" size="small" type="primary" link @click="addSequenceItem(member)">+</el-button>
        </div>
        <div class="member-value">
          <!-- 基本类型 -->
          <el-input v-if="!member.isSequence && !member.isStruct" v-model="member.value" size="small" />
          <!-- 结构体简化展示 -->
          <div v-else-if="member.isStruct" class="struct-fields">
            <div v-for="(field, idx) in member.structFields" :key="idx" class="struct-field">
              <span>{{ field.name }}:</span>
              <el-input v-model="field.value" size="small" />
            </div>
          </div>
          <!-- sequence 列表 -->
          <div v-else-if="member.isSequence" class="sequence-list">
            <div v-for="(item, idx) in member.sequenceValue" :key="idx" class="sequence-item">
              <el-input v-model="item.value" size="small" />
              <el-button size="small" type="danger" link @click="removeSequenceItem(member, idx)">删除</el-button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, watch, computed } from 'vue'
import type { Node } from '@vue-flow/core'
import { ElInput, ElButton } from 'element-plus'

const props = defineProps<{
  selectedNode: Node | null
}>()

interface MemberConfig {
  name: string
  typeName: string
  isSequence: boolean
  isStruct: boolean
  value: any
  sequenceValue: any[]
  structFields: { name: string; value: any }[]
}

const membersList = ref<MemberConfig[]>([])

// 根据选中节点的model里的members解析成员列表
const parseMembersFromNode = (node: Node) => {
  const model = node.data?.model
  console.log('Parsing members from node:', model)
  if (!model || !model.members) return []
  return model.members.map((member: any) => {
    const typeInfo = member.typeInfo || {}
    const isSequence = member.sequence_type === 'sequence' || member.array === 'sequence'
    const isStruct = typeInfo.kind === 'struct'
    return {
      name: member.name,
      typeName: member.display_type || member.type,
      isSequence,
      isStruct,
      value: member.defaultValue ?? '',
      sequenceValue: member.defaultSequence ? member.defaultSequence.map((v: any) => ({ value: v })) : [{ value: '' }],
      structFields: isStruct ? (typeInfo.fields?.map((f: any) => ({ name: f.name, value: '' })) || []) : [],
    }
  })
}

const addSequenceItem = (member: MemberConfig) => {
  member.sequenceValue.push({ value: '' })
}

const removeSequenceItem = (member: MemberConfig, idx: number) => {
  member.sequenceValue.splice(idx, 1)
}

watch(() => props.selectedNode, (newNode) => {
  if (newNode) {
    membersList.value = parseMembersFromNode(newNode)
  } else {
    membersList.value = []
  }
}, { immediate: true, deep: true })
</script>

<style scoped>
.members-config {
  height: 100%;
  overflow-y: auto;
}
h4 {
  margin: 0 0 12px 0;
}
.empty-tip {
  color: #909399;
  text-align: center;
  margin-top: 40px;
}
.member-item {
  border-bottom: 1px solid #e4e7ed;
  padding: 12px 0;
}
.member-header {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 8px;
}
.member-name {
  font-weight: bold;
  font-size: 13px;
}
.member-type {
  font-size: 12px;
  color: #909399;
}
.struct-fields {
  margin-left: 16px;
  border-left: 2px solid #dcdfe6;
  padding-left: 12px;
}
.struct-field {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 8px;
}
.sequence-list {
  margin-top: 8px;
}
.sequence-item {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 8px;
}
</style>