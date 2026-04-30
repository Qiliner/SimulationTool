<template>
  <div class="model-designer">
    <!-- 动态背景装饰 -->
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 顶部标题栏 -->
    <div class="header glass-panel">
      <div class="header-left">
        <div class="logo-icon">
          <svg width="28" height="28" viewBox="0 0 24 24" fill="none">
            <path d="M4 4H20V20H4V4Z" stroke="currentColor" stroke-width="1.5" rx="2"/>
            <path d="M8 2V6M16 2V6M4 10H20" stroke="currentColor" stroke-width="1.5"/>
            <path d="M12 14L12 18M9 15L15 15" stroke="currentColor" stroke-width="1.5"/>
          </svg>
        </div>
        <div class="header-text">
          <h1>模型设计器</h1>
          <span class="subtitle">数据结构与模型可视化设计 | 实时配置</span>
        </div>
      </div>
      <div class="header-right">
        <div class="import-export-group">
          <button class="btn-generate" @click="openGenerateDialog">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M13 2L3 14H12L11 22L21 10H12L13 2Z"/>
            </svg>
            代码生成
          </button>
          <button class="btn-import" @click="importFromXML">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M12 3v12m0 0-3-3m3 3 3-3M5 17v2a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-2"/>
            </svg>
            导入XML
          </button>
          <button class="btn-export" @click="exportToXML">
            <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M12 3v12m0 0-3-3m3 3 3-3M5 17v2a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-2"/>
            </svg>
            导出XML
          </button>
        </div>
        <div class="stats-badge">
          <span class="stats-dot" :class="{ active: activeTab === 'data' }"></span>
          <span class="stats-text">{{ activeTab === 'data' ? '数据结构' : '模型设计' }}</span>
          <span class="stats-number">{{ activeTab === 'data' ? dataTypes.length : models.length }}</span>
        </div>
      </div>
    </div>

    <!-- 主布局 -->
    <div class="main-layout">
      <!-- 左侧列表 -->
      <div class="left-sidebar glass-panel">
        <div class="sidebar-header">
          <div class="tabs">
            <button 
              :class="['tab', { active: activeTab === 'data' }]" 
              @click="activeTab = 'data'"
            >
              📊 数据结构
            </button>
            <button 
              :class="['tab', { active: activeTab === 'model' }]" 
              @click="activeTab = 'model'"
            >
              🧠 模型设计
            </button>
          </div>
        </div>

        <!-- 数据结构列表 -->
        <div v-if="activeTab === 'data'" class="list-container">
          <div class="list-header">
            <span class="list-title">数据结构集群</span>
            <button class="btn-add" @click="createDataType">+ 新建</button>
          </div>
          <div class="list-items">
            <div
              v-for="item in dataTypes"
              :key="item.id"
              class="list-item"
              :class="{ active: selectedDataType?.id === item.id }"
              @click="selectDataType(item)"
            >
              <div class="item-icon">
                <div class="icon-bg data">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                    <rect x="3" y="3" width="18" height="18" rx="2" stroke="currentColor" stroke-width="1.5"/>
                    <path d="M8 7H16M8 12H16M8 17H12" stroke="currentColor" stroke-width="1.5"/>
                  </svg>
                </div>
              </div>
              <div class="item-info">
                <div class="item-name">{{ item.name || '未命名' }}</div>
                <div class="item-desc">{{ item.description || '无描述' }}</div>
              </div>
              <div class="item-actions">
                <button class="btn-icon edit" @click.stop="editDataType(item)">✎</button>
                <button class="btn-icon delete" @click.stop="deleteDataType(item.id)">×</button>
              </div>
            </div>
            <div v-if="dataTypes.length === 0" class="empty-list">
              <div class="empty-illustration">
                <svg width="64" height="64" viewBox="0 0 24 24" fill="none">
                  <path d="M12 8V12L15 15" stroke="currentColor" stroke-width="1.5"/>
                  <circle cx="12" cy="12" r="9" stroke="currentColor" stroke-width="1.5"/>
                </svg>
              </div>
              <p>暂无数据结构</p>
              <span>点击"新建"创建第一个数据结构</span>
            </div>
          </div>
        </div>

        <!-- 模型列表 -->
        <div v-if="activeTab === 'model'" class="list-container">
          <div class="list-header">
            <span class="list-title">模型集群</span>
            <button class="btn-add" @click="createModel">+ 新建</button>
          </div>
          <div class="list-items">
            <div
              v-for="model in models"
              :key="model.id"
              class="list-item"
              :class="{ active: selectedModel?.id === model.id }"
              @click="selectModel(model)"
            >
              <div class="item-icon">
                <div class="icon-bg model">
                  <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                    <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.5"/>
                    <path d="M2 17L12 22L22 17" stroke="currentColor" stroke-width="1.5"/>
                    <path d="M2 12L12 17L22 12" stroke="currentColor" stroke-width="1.5"/>
                  </svg>
                </div>
              </div>
              <div class="item-info">
                <div class="item-name">{{ model.displayName || model.name || '未命名' }}</div>
                <div class="item-desc">模型名: {{ model.name }}</div>
              </div>
              <div class="item-actions">
                <button class="btn-icon edit" @click.stop="editModel(model)">✎</button>
                <button class="btn-icon delete" @click.stop="deleteModel(model.id)">×</button>
              </div>
            </div>
            <div v-if="models.length === 0" class="empty-list">
              <div class="empty-illustration">
                <svg width="64" height="64" viewBox="0 0 24 24" fill="none">
                  <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.5"/>
                  <path d="M2 17L12 22L22 17" stroke="currentColor" stroke-width="1.5"/>
                  <path d="M2 12L12 17L22 12" stroke="currentColor" stroke-width="1.5"/>
                </svg>
              </div>
              <p>暂无模型</p>
              <span>点击"新建"创建第一个模型</span>
            </div>
          </div>
        </div>
      </div>

      <!-- 右侧编辑区域 -->
      <div class="right-content">
        <!-- 数据结构编辑 -->
        <div v-if="activeTab === 'data' && selectedDataType && editingDataType" class="detail-card glass-panel">
          <div class="detail-header">
            <div class="header-title-group">
              <div class="type-badge data">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                  <rect x="3" y="3" width="18" height="18" rx="2" stroke="currentColor" stroke-width="1.5"/>
                </svg>
                <h2>数据结构配置</h2>
              </div>
              <span class="status-chip">编辑中</span>
            </div>
          </div>

          <div class="form-group">
            <label>名称 * (英文)</label>
            <input v-model="editingDataType.name" placeholder="例如: Vector3" class="form-input" />
          </div>
          <div class="form-group">
            <label>描述</label>
            <textarea v-model="editingDataType.description" rows="2" placeholder="数据结构的描述" class="form-textarea"></textarea>
          </div>

          <div class="members-section">
            <div class="section-header">
              <div class="section-title">
                <span>📋 成员列表</span>
                <span class="section-count">{{ editingDataType.members?.length || 0 }} 个成员</span>
              </div>
              <button class="btn-secondary" @click="addMember">+ 添加成员</button>
            </div>
            
            <div class="members-table-wrapper">
              <table class="members-table">
                <thead>
                  <tr>
                    <th style="width: 40px">#</th>
                    <th>显示名</th>
                    <th>数据类型</th>
                    <th>子类型</th>
                    <th>变量名</th>
                    <th>范围</th>
                    <th>默认值</th>
                    <th>单位</th>
                    <th>说明</th>
                    <th style="width: 120px">操作</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="(member, idx) in editingDataType.members" :key="idx">
                    <td class="index-cell">{{ idx + 1 }}</td>
                    <td>{{ member.displayName || '-' }}</td>
                    <td>
                      <span class="type-tag" :class="{ 'type-sequence': member.type === 'sequence' }">
                        {{ member.type || 'int32' }}
                      </span>
                    </td>
                    <td>
                      <div v-if="member.type === 'sequence'" class="subtype-cell">
                        <span class="subtype-preview">sequence&lt;{{ member.subType || '?' }}&gt;</span>
                        <select v-model="member.subType" class="subtype-select">
                          <option value="">请选择子类型</option>
                          <optgroup label="基础类型">
                            <option value="int8">int8</option><option value="int16">int16</option>
                            <option value="int32">int32</option><option value="int64">int64</option>
                            <option value="uint8">uint8</option><option value="uint16">uint16</option>
                            <option value="uint32">uint32</option><option value="uint64">uint64</option>
                            <option value="real32">real32</option><option value="real64">real64</option>
                            <option value="bool">bool</option><option value="string">string</option>
                          </optgroup>
                          <optgroup label="自定义类型">
                            <option v-for="dt in getAvailableSubTypes(editingDataType?.name)" :key="dt.id" :value="dt.name">{{ dt.name }}</option>
                          </optgroup>
                        </select>
                      </div>
                      <span v-else class="subtype-placeholder">-</span>
                    </td>
                    <td>{{ member.varName || '-' }}</td>
                    <td>{{ member.range || '-' }}</td>
                    <td>{{ member.defaultValue || '-' }}</td>
                    <td>{{ member.unit || '-' }}</td>
                    <td class="desc-cell">{{ member.description || '-' }}</td>
                    <td>
                      <div class="table-actions">
                        <button class="table-btn move-up" @click="moveMemberUp(idx)" :disabled="idx === 0">↑</button>
                        <button class="table-btn move-down" @click="moveMemberDown(idx)" :disabled="idx === editingDataType.members.length - 1">↓</button>
                        <button class="table-btn edit" @click="openMemberEditor(idx)">✎</button>
                        <button class="table-btn delete" @click="removeMember(idx)">×</button>
                      </div>
                    </td>
                  </tr>
                  <tr v-if="!editingDataType.members || editingDataType.members.length === 0">
                    <td colspan="10" class="empty-table">暂无成员，点击"添加成员"按钮添加</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>

          <div class="editor-actions">
            <button class="btn-primary" @click="saveDataType" :disabled="isLoading">💾 保存数据结构</button>
            <button class="btn-secondary" @click="cancelEditDataType">取消</button>
          </div>
        </div>

        <!-- 模型编辑 -->
        <div v-if="activeTab === 'model' && selectedModel && editingModel" class="detail-card glass-panel">
          <div class="detail-header">
            <div class="header-title-group">
              <div class="type-badge model">
                <svg width="20" height="20" viewBox="0 0 24 24" fill="none">
                  <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.5"/>
                </svg>
                <h2>模型配置</h2>
              </div>
              <span class="status-chip">编辑中</span>
            </div>
          </div>

          <div class="form-group">
            <label>模型名称 * (英文，用于代码生成)</label>
            <input v-model="editingModel.name" placeholder="例如: Send" class="form-input" />
          </div>
          <div class="form-group">
            <label>显示名 (中文)</label>
            <input v-model="editingModel.displayName" placeholder="例如: 发送模型" class="form-input" />
          </div>

          <div class="interface-tabs">
            <button 
              v-for="typeName in interfaceTypes" 
              :key="typeName" 
              :class="['interface-tab', { active: currentInterfaceType === typeName }]" 
              @click="currentInterfaceType = typeName"
            >
              {{ typeName }}
            </button>
          </div>

          <div class="interfaces-section">
            <div class="section-header">
              <div class="section-title">
                <span>{{ currentInterfaceType }}列表</span>
                <span class="section-count">{{ getCurrentInterfaces().length }} 个接口</span>
              </div>
              <button class="btn-secondary" @click="addInterface(currentInterfaceType)">+ 添加{{ currentInterfaceType }}</button>
            </div>
            
            <div class="interfaces-table-wrapper">
              <table class="interfaces-table">
                <thead>
                  <tr>
                    <th style="width: 40px">#</th>
                    <th>接口名称</th>
                    <th>数据类型</th>
                    <th v-if="needTopic(currentInterfaceType)">主题名</th>
                    <th>显示名</th>
                    <th>说明</th>
                    <th style="width: 100px">操作</th>
                  </tr>
                </thead>
                <tbody>
                  <tr v-for="(iface, idx) in getCurrentInterfaces()" :key="idx">
                    <td class="index-cell">{{ idx + 1 }}</td>
                    <td><input v-model="iface.name" placeholder="接口名称" class="table-input" /></td>
                    <td>
                      <select v-model="iface.dataType" class="table-select">
                        <optgroup label="基础类型">
                          <option>int8</option><option>int16</option><option>int32</option><option>int64</option>
                          <option>uint8</option><option>uint16</option><option>uint32</option><option>uint64</option>
                          <option>real32</option><option>real64</option>
                          <option>bool</option><option>string</option>
                        </optgroup>
                        <optgroup label="自定义类型">
                          <option v-for="dataType in dataTypes" :key="dataType.id" :value="dataType.name">{{ dataType.name }}</option>
                        </optgroup>
                      </select>
                    </td>
                    <td v-if="needTopic(currentInterfaceType)">
                      <input v-model="iface.topic" placeholder="主题名" class="table-input" />
                    </td>
                    <td><input v-model="iface.displayName" placeholder="显示名" class="table-input" /></td>
                    <td><input v-model="iface.description" placeholder="说明" class="table-input" /></td>
                    <td>
                      <div class="table-actions">
                        <button class="table-btn move-up" @click="moveInterfaceUp(currentInterfaceType, idx)" :disabled="idx === 0">↑</button>
                        <button class="table-btn move-down" @click="moveInterfaceDown(currentInterfaceType, idx)" :disabled="idx === getCurrentInterfaces().length - 1">↓</button>
                        <button class="table-btn delete" @click="removeInterface(currentInterfaceType, idx)">×</button>
                      </div>
                    </td>
                  </tr>
                  <tr v-if="getCurrentInterfaces().length === 0">
                    <td :colspan="needTopic(currentInterfaceType) ? 7 : 6" class="empty-table">暂无{{ currentInterfaceType }}，点击"添加"按钮添加</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>

          <div class="editor-actions">
            <button class="btn-primary" @click="saveModel" :disabled="isLoading">💾 保存模型</button>
            <button class="btn-secondary" @click="cancelEditModel">取消</button>
          </div>
        </div>

        <!-- 未选择提示 -->
        <div v-else-if="(activeTab === 'data' && (!selectedDataType || !editingDataType)) || (activeTab === 'model' && (!selectedModel || !editingModel))" class="empty-detail glass-panel">
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
          <p>选择项目查看详细配置</p>
          <span class="empty-hint">点击左侧卡片选择要编辑的数据结构或模型</span>
        </div>
      </div>
    </div>

    <!-- 成员编辑弹窗 -->
    <div v-if="editingMemberIndex !== null" class="modal-overlay" @click.self="closeMemberEditor">
      <div class="modal-content glass-panel">
        <div class="modal-header">
          <h3>编辑成员</h3>
          <button class="modal-close" @click="closeMemberEditor">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>显示名</label>
            <input v-model="currentEditingMember.displayName" class="form-input" />
          </div>
          <div class="form-group">
            <label>数据类型</label>
            <select v-model="currentEditingMember.type" class="form-select" @change="onMemberTypeChange">
              <optgroup label="基础类型">
                <option>int8</option><option>int16</option><option>int32</option><option>int64</option>
                <option>uint8</option><option>uint16</option><option>uint32</option><option>uint64</option>
                <option>real32</option><option>real64</option>
                <option>bool</option><option>string</option>
              </optgroup>
              <optgroup label="容器类型">
                <option>sequence</option>
              </optgroup>
              <optgroup label="自定义类型">
                <option v-for="dt in getAvailableSubTypes(editingDataType?.name)" :key="dt.id" :value="dt.name">{{ dt.name }}</option>
              </optgroup>
            </select>
          </div>
          <div class="form-group" v-if="currentEditingMember.type === 'sequence'">
            <label>子类型 (sequence&lt;子类型&gt;)</label>
            <select v-model="currentEditingMember.subType" class="form-select">
              <option value="">请选择子类型</option>
              <optgroup label="基础类型">
                <option value="int8">int8</option><option value="int16">int16</option>
                <option value="int32">int32</option><option value="int64">int64</option>
                <option value="uint8">uint8</option><option value="uint16">uint16</option>
                <option value="uint32">uint32</option><option value="uint64">uint64</option>
                <option value="real32">real32</option><option value="real64">real64</option>
                <option value="bool">bool</option><option value="string">string</option>
              </optgroup>
              <optgroup label="自定义类型">
                <option v-for="dt in getAvailableSubTypes(editingDataType?.name)" :key="dt.id" :value="dt.name">{{ dt.name }}</option>
              </optgroup>
            </select>
            <div class="subtype-hint" v-if="currentEditingMember.subType">
              <code>sequence&lt;{{ currentEditingMember.subType }}&gt;</code>
            </div>
          </div>
          <div class="form-group">
            <label>变量名</label>
            <input v-model="currentEditingMember.varName" class="form-input" />
          </div>
          <div class="form-group">
            <label>范围</label>
            <input v-model="currentEditingMember.range" class="form-input" />
          </div>
          <div class="form-group">
            <label>默认值</label>
            <input v-model="currentEditingMember.defaultValue" class="form-input" />
          </div>
          <div class="form-group">
            <label>单位</label>
            <input v-model="currentEditingMember.unit" class="form-input" />
          </div>
          <div class="form-group">
            <label>说明</label>
            <textarea v-model="currentEditingMember.description" rows="2" class="form-textarea"></textarea>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="saveMemberEdit">保存</button>
          <button class="btn-secondary" @click="closeMemberEditor">取消</button>
        </div>
      </div>
    </div>

    <!-- 代码生成弹窗 -->
    <div v-if="showGenerateDialog" class="modal-overlay" @click.self="showGenerateDialog = false">
      <div class="modal-content glass-panel" style="width: 500px;">
        <div class="modal-header">
          <h3>⚡ 代码生成</h3>
          <button class="modal-close" @click="showGenerateDialog = false">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>选择模型</label>
            <select v-model="selectedModelForGenerate" class="form-select">
              <option value="">请选择模型</option>
              <option v-for="model in models" :key="model.id" :value="model.name">{{ model.displayName || model.name }}</option>
            </select>
          </div>
          <div class="form-group">
            <label>选择语言</label>
            <select v-model="selectedLanguage" class="form-select">
              <option value="c">C</option>
              <option value="c++">C++</option>
              <option value="matlab">MATLAB</option>
              <option value="python">Python</option>
            </select>
          </div>
          <div class="generate-preview" v-if="selectedModelForGenerate">
            <div class="preview-title">将生成的模型: {{ selectedModelForGenerate }}</div>
            <div class="preview-info">
              <span>关联数据类型: {{ getRelatedTypes(selectedModelForGenerate).length }} 个</span>
            </div>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="generateCode" :disabled="!selectedModelForGenerate || isGenerating">
            🚀 生成并下载
          </button>
          <button class="btn-secondary" @click="showGenerateDialog = false">取消</button>
        </div>
      </div>
    </div>

    <!-- 加载提示 -->
   <!-- 在加载提示中添加生成进度 -->
    <div v-if="isGenerating" class="loading-overlay">
        <div class="loading-spinner">
            <div class="spinner"></div>
            <p>正在生成代码，请稍候...</p>
            <p class="progress-text" v-if="generateProgress">{{ generateProgress }}</p>
        </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, reactive, toRaw, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'

interface DataTypeMember {
  displayName: string
  type: string
  subType?: string
  range: string
  varName: string
  defaultValue: string
  unit: string
  description: string
}

interface DataType {
  id: number | null
  name: string
  description: string
  members: DataTypeMember[]
  uuid?: string
}

interface ModelInterface {
  name: string
  dataType: string
  topic?: string
  displayName: string
  description: string
}

interface Model {
  id: number | null
  name: string
  displayName: string
  paramInterfaces: ModelInterface[]
  inputInterfaces: ModelInterface[]
  outputInterfaces: ModelInterface[]
  initInterfaces: ModelInterface[]
  eventInterfaces: ModelInterface[]
  uuid?: string
}

const API_BASE_URL = 'http://192.168.156.20:8080/api'
const WS_URL = 'ws://192.168.156.20:8081'

const router = useRouter()
const userStore = useUserStore()

const getHeaders = () => {
    return {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${userStore.token}`,
        'X-Project-Id': currentProject.value?.id?.toString() || ''
    }
}

const dataTypes = ref<DataType[]>([])
let nextDataTypeId = 1

const models = ref<Model[]>([])
let nextModelId = 1

const activeTab = ref<'data' | 'model'>('data')
const selectedDataType = ref<DataType | null>(null)
const selectedModel = ref<Model | null>(null)
const editingDataType = ref<DataType | null>(null)
const editingModel = ref<Model | null>(null)
const currentInterfaceType = ref('参数接口')

const editingMemberIndex = ref<number | null>(null)
const currentEditingMember = ref<DataTypeMember | null>(null)

const showGenerateDialog = ref(false)
const isGenerating = ref(false)
const isLoading = ref(false)
const selectedModelForGenerate = ref('')
const selectedLanguage = ref('python')

const openGenerateDialog = () => {
  selectedModelForGenerate.value = ''
  selectedLanguage.value = 'python'
  showGenerateDialog.value = true
}

let ws: WebSocket | null = null

const interfaceTypes = ['参数接口', '输入接口', '输出接口', '初始化接口', '事件接口']
const baseTypes = ['int8', 'int16', 'int32', 'int64', 'uint8', 'uint16', 'uint32', 'uint64', 'real32', 'real64', 'bool', 'string']

const needTopic = (type: string) => {
  return type === '输入接口' || type === '输出接口'
}

const getCurrentInterfaces = () => {
  if (!editingModel.value) return []
  const map: Record<string, keyof Model> = {
    '参数接口': 'paramInterfaces',
    '输入接口': 'inputInterfaces',
    '输出接口': 'outputInterfaces',
    '初始化接口': 'initInterfaces',
    '事件接口': 'eventInterfaces'
  }
  const key = map[currentInterfaceType.value]
  return editingModel.value[key] || []
}

const getAvailableSubTypes = (currentDataTypeName?: string | null) => {
  return dataTypes.value.filter(dt => dt.name !== currentDataTypeName)
}

const getRelatedTypes = (modelName: string): DataType[] => {
  const model = models.value.find(m => m.name === modelName)
  if (!model) return []
  
  const usedTypeNames = new Set<string>()
  
  const collectTypes = (interfaces: ModelInterface[]) => {
    interfaces.forEach(iface => {
      usedTypeNames.add(iface.dataType)
    })
  }
  
  collectTypes(model.paramInterfaces)
  collectTypes(model.inputInterfaces)
  collectTypes(model.outputInterfaces)
  collectTypes(model.initInterfaces)
  collectTypes(model.eventInterfaces)
  
  const result: DataType[] = []
  const processed = new Set<string>()
  
  const addTypeAndDependencies = (typeName: string) => {
    if (processed.has(typeName)) return
    processed.add(typeName)
    
    const dataType = dataTypes.value.find(dt => dt.name === typeName)
    if (dataType) {
      result.push(dataType)
      dataType.members.forEach(member => {
        if (!baseTypes.includes(member.type) && member.type !== 'sequence') {
          addTypeAndDependencies(member.type)
        }
        if (member.type === 'sequence' && member.subType && !baseTypes.includes(member.subType)) {
          addTypeAndDependencies(member.subType)
        }
      })
    }
  }
  
  usedTypeNames.forEach(typeName => {
    if (!baseTypes.includes(typeName)) {
      addTypeAndDependencies(typeName)
    }
  })
  
  return result
}

const generateUUID = (): string => {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, (c) => {
    const r = Math.random() * 16 | 0
    const v = c === 'x' ? r : (r & 0x3 | 0x8)
    return v.toString(16)
  })
}

const getDataTypeUUID = (typeName: string): string => {
  const dataType = dataTypes.value.find(dt => dt.name === typeName)
  if (dataType) {
    if (!dataType.uuid) {
      dataType.uuid = generateUUID()
    }
    return dataType.uuid
  }
  return typeName
}

const getInterfaceTypeRef = (dataTypeName: string): string => {
  if (!baseTypes.includes(dataTypeName)) {
    return getDataTypeUUID(dataTypeName)
  }
  return dataTypeName
}

// ==================== API 调用函数 ====================

const fetchDataTypesFromServer = async () => {
  try {
    const response = await fetch(`${API_BASE_URL}/datatypes`, {
      headers: getHeaders()
    })
    if (!response.ok) throw new Error('Failed to fetch data types')
    const data = await response.json()
    dataTypes.value = data
    if (data.length > 0) {
      const maxId = Math.max(...data.map((dt: DataType) => dt.id || 0))
      nextDataTypeId = maxId + 1
    }
    return data
  } catch (error) {
    console.error('Failed to fetch data types:', error)
    return []
  }
}

const fetchModelsFromServer = async () => {
  try {
    const response = await fetch(`${API_BASE_URL}/models`, {
      headers: getHeaders()
    })
    if (!response.ok) throw new Error('Failed to fetch models')
    const data = await response.json()
    models.value = data
    if (data.length > 0) {
      const maxId = Math.max(...data.map((m: Model) => m.id || 0))
      nextModelId = maxId + 1
    }
    return data
  } catch (error) {
    console.error('Failed to fetch models:', error)
    return []
  }
}

const saveDataTypeToServer = async (dataType: DataType): Promise<DataType | null> => {
  try {
    const isNew = !dataType.id
    const url = isNew ? `${API_BASE_URL}/datatypes` : `${API_BASE_URL}/datatypes/${dataType.id}`
    const method = isNew ? 'POST' : 'PUT'
    
    const response = await fetch(url, {
      method,
      headers: getHeaders(),
      body: JSON.stringify(dataType)
    })
    
    if (!response.ok) throw new Error(`Failed to ${isNew ? 'create' : 'update'} data type`)
    const result = await response.json()
    return result.data || dataType
  } catch (error) {
    console.error('Failed to save data type:', error)
    alert('保存数据结构失败')
    return null
  }
}

const deleteDataTypeFromServer = async (id: number): Promise<boolean> => {
  try {
    const response = await fetch(`${API_BASE_URL}/datatypes/${id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    if (!response.ok) throw new Error('Failed to delete data type')
    return true
  } catch (error) {
    console.error('Failed to delete data type:', error)
    alert('删除数据结构失败')
    return false
  }
}

const saveModelToServer = async (model: Model): Promise<Model | null> => {
  try {
    const isNew = !model.id
    const url = isNew ? `${API_BASE_URL}/models` : `${API_BASE_URL}/models/${model.id}`
    const method = isNew ? 'POST' : 'PUT'
    
    const response = await fetch(url, {
      method,
      headers: getHeaders(),
      body: JSON.stringify(model)
    })
    
    if (!response.ok) throw new Error(`Failed to ${isNew ? 'create' : 'update'} model` + model.id)
    const result = await response.json()
    return result.data || model
  } catch (error) {
    console.error('Failed to save model:', error)
    alert('保存模型失败')
    return null
  }
}

const deleteModelFromServer = async (id: number): Promise<boolean> => {
  try {
    const response = await fetch(`${API_BASE_URL}/models/${id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    if (!response.ok) throw new Error('Failed to delete model')
    return true
  } catch (error) {
    console.error('Failed to delete model:', error)
    alert('删除模型失败')
    return false
  }
}

const syncAllToServer = async () => {
  isLoading.value = true
  try {
    const response = await fetch(`${API_BASE_URL}/sync`, {
      method: 'POST',
      headers: getHeaders(),
      body: JSON.stringify({
        dataTypes: dataTypes.value,
        models: models.value
      })
    })
    if (!response.ok) throw new Error('Failed to sync data')
    alert('同步成功！')
  } catch (error) {
    console.error('Failed to sync:', error)
    alert('同步失败！')
  } finally {
    isLoading.value = false
  }
}

// ==================== WebSocket ====================

const handleWebSocketMessage = (message: any) => {
  const { event, data } = message
  
  switch (event) {
    case 'data_type_created':
    case 'data_type_updated':
      const index = dataTypes.value.findIndex(dt => dt.id === data.id)
      if (index !== -1) {
        dataTypes.value[index] = data
      } else {
        dataTypes.value.push(data)
      }
      break
      
    case 'data_type_deleted':
      dataTypes.value = dataTypes.value.filter(dt => dt.id !== data)
      if (selectedDataType.value?.id === data) {
        selectedDataType.value = null
        editingDataType.value = null
      }
      break
      
    case 'model_created':
    case 'model_updated':
      const modelIndex = models.value.findIndex(m => m.id === data.id)
      if (modelIndex !== -1) {
        models.value[modelIndex] = data
      } else {
        models.value.push(data)
      }
      break
      
    case 'model_deleted':
      models.value = models.value.filter(m => m.id !== data)
      if (selectedModel.value?.id === data) {
        selectedModel.value = null
        editingModel.value = null
      }
      break
  }
}

const setupWebSocket = () => {
  try {
    ws = new WebSocket(WS_URL)
    
    ws.onopen = () => {
      console.log('WebSocket connected')
    }
    
    ws.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data)
        handleWebSocketMessage(message)
      } catch (e) {
        console.error('Failed to parse WebSocket message:', e)
      }
    }
    
    ws.onerror = (error) => {
      console.error('WebSocket error:', error)
    }
    
    ws.onclose = () => {
      console.log('WebSocket disconnected, reconnecting in 5 seconds...')
      setTimeout(setupWebSocket, 5000)
    }
  } catch (error) {
    console.error('Failed to setup WebSocket:', error)
  }
}

// ==================== 数据结构操作 ====================

const createDataType = () => {
  const newDataType: DataType = {
    id: null,
    name: '',
    description: '',
    members: []
  }
  const reactiveData = reactive(JSON.parse(JSON.stringify(newDataType)))
  editingDataType.value = reactiveData
  selectedDataType.value = reactiveData
}

const selectDataType = (dt: DataType) => {
  if (!dt) return
  selectedDataType.value = dt
  const clonedData = JSON.parse(JSON.stringify(dt))
  editingDataType.value = reactive(clonedData)
}

const editDataType = (dt: DataType) => {
  selectDataType(dt)
}

const deleteDataType = async (id: number | null) => {
    if (!id) return
    if (!confirm('确定要删除这个数据结构吗？')) return
    
    isLoading.value = true
    
    try {
        const success = await deleteDataTypeFromServer(id)
        if (success) {
            dataTypes.value = dataTypes.value.filter(dt => dt.id !== id)
            if (selectedDataType.value?.id === id) {
                selectedDataType.value = null
                editingDataType.value = null
            }
            alert('数据结构删除成功')
            
            // 保存工程数据
            await saveProjectData()
        }
    } finally {
        isLoading.value = false
    }
}

const saveDataType = async () => {
    if (!editingDataType.value) return
    if (!editingDataType.value.name || !editingDataType.value.name.trim()) {
        alert('数据结构名称不能为空')
        return
    }
    
    const exists = dataTypes.value.some(dt => dt.name === editingDataType.value?.name && dt.id !== editingDataType.value?.id)
    if (exists) {
        alert('数据结构名称已存在')
        return
    }
    
    isLoading.value = true
    
    try {
        const dataToSave = JSON.parse(JSON.stringify(toRaw(editingDataType.value)))
        if (!dataToSave.uuid) {
            dataToSave.uuid = generateUUID()
        }
        
        const savedDataType = await saveDataTypeToServer(dataToSave)
        
        if (savedDataType) {
            if (editingDataType.value.id) {
                const index = dataTypes.value.findIndex(dt => dt.id === editingDataType.value?.id)
                if (index !== -1) {
                    dataTypes.value[index] = savedDataType
                }
            } else {
                dataTypes.value.push(savedDataType)
                editingDataType.value.id = savedDataType.id
            }
            
            selectedDataType.value = editingDataType.value
            editingDataType.value = reactive(JSON.parse(JSON.stringify(savedDataType)))
            alert('数据结构保存成功')
            
            // 保存工程数据
            await saveProjectData()
        }
    } finally {
        isLoading.value = false
    }
}


const cancelEditDataType = () => {
  if (selectedDataType.value) {
    editingDataType.value = reactive(JSON.parse(JSON.stringify(selectedDataType.value)))
  } else {
    editingDataType.value = null
    selectedDataType.value = null
  }
}

// ==================== 模型操作 ====================

const createModel = () => {
  const newModel: Model = {
    id: null,
    name: '',
    displayName: '',
    paramInterfaces: [],
    inputInterfaces: [],
    outputInterfaces: [],
    initInterfaces: [],
    eventInterfaces: []
  }
  editingModel.value = reactive(JSON.parse(JSON.stringify(newModel)))
  selectedModel.value = editingModel.value
  currentInterfaceType.value = '参数接口'
}

const selectModel = (model: Model) => {
  if (!model) return
  selectedModel.value = model
  editingModel.value = reactive(JSON.parse(JSON.stringify(model)))
}

const editModel = (model: Model) => {
  selectModel(model)
}
const deleteModel = async (id: number | null) => {
    if (!id) return
    if (!confirm('确定要删除这个模型吗？')) return
    
    isLoading.value = true
    
    try {
        const success = await deleteModelFromServer(id)
        if (success) {
            models.value = models.value.filter(m => m.id !== id)
            if (selectedModel.value?.id === id) {
                selectedModel.value = null
                editingModel.value = null
            }
            alert('模型删除成功')
            
            // 保存工程数据
            await saveProjectData()
        }
    } finally {
        isLoading.value = false
    }
}

const saveModel = async () => {
    if (!editingModel.value) return
    if (!editingModel.value.name.trim()) {
        alert('模型名称不能为空')
        return
    }
    
    const exists = models.value.some(m => m.name === editingModel.value?.name && m.id !== editingModel.value?.id)
    if (exists) {
        alert('模型名称已存在')
        return
    }
    
    isLoading.value = true
    
    try {
        const dataToSave = JSON.parse(JSON.stringify(toRaw(editingModel.value)))
        if (!dataToSave.uuid) {
            dataToSave.uuid = generateUUID()
        }
        
        const savedModel = await saveModelToServer(dataToSave)
        
        if (savedModel) {
            if (editingModel.value.id) {
                const index = models.value.findIndex(m => m.id === editingModel.value?.id)
                if (index !== -1) {
                    models.value[index] = savedModel
                }
            } else {
                models.value.push(savedModel)
                editingModel.value.id = savedModel.id
            }
            
            selectedModel.value = editingModel.value
            editingModel.value = reactive(JSON.parse(JSON.stringify(savedModel)))
            alert('模型保存成功')
            
            // 保存工程数据
            await saveProjectData()
        }
    } finally {
        isLoading.value = false
    }
}

const cancelEditModel = () => {
  if (selectedModel.value) {
    editingModel.value = reactive(JSON.parse(JSON.stringify(selectedModel.value)))
  } else {
    editingModel.value = null
    selectedModel.value = null
  }
}

// ==================== 成员操作 ====================

const addMember = () => {
  if (!editingDataType.value) return
  if (!editingDataType.value.members) {
    editingDataType.value.members = []
  }
  const newMember: DataTypeMember = {
    displayName: '',
    type: 'int32',
    subType: undefined,
    range: '',
    varName: '',
    defaultValue: '',
    unit: '',
    description: ''
  }
  editingDataType.value.members.push(newMember)
  const newIdx = editingDataType.value.members.length - 1
  openMemberEditor(newIdx)
}

const openMemberEditor = (idx: number) => {
  if (!editingDataType.value || !editingDataType.value.members || !editingDataType.value.members[idx]) return
  editingMemberIndex.value = idx
  currentEditingMember.value = reactive(JSON.parse(JSON.stringify(editingDataType.value.members[idx])))
}

const closeMemberEditor = () => {
  editingMemberIndex.value = null
  currentEditingMember.value = null
}

const saveMemberEdit = () => {
  if (editingMemberIndex.value !== null && currentEditingMember.value && editingDataType.value && editingDataType.value.members) {
    if (currentEditingMember.value.type !== 'sequence') {
      currentEditingMember.value.subType = undefined
    }
    Object.assign(editingDataType.value.members[editingMemberIndex.value], currentEditingMember.value)
    closeMemberEditor()
  }
}

const removeMember = (idx: number) => {
  if (!editingDataType.value || !editingDataType.value.members) return
  editingDataType.value.members.splice(idx, 1)
  if (editingMemberIndex.value === idx) {
    closeMemberEditor()
  } else if (editingMemberIndex.value !== null && editingMemberIndex.value > idx) {
    editingMemberIndex.value--
  }
}

const moveMemberUp = (idx: number) => {
  if (!editingDataType.value || idx === 0) return
  const members = editingDataType.value.members
  ;[members[idx - 1], members[idx]] = [members[idx], members[idx - 1]]
}

const moveMemberDown = (idx: number) => {
  if (!editingDataType.value || idx === editingDataType.value.members.length - 1) return
  const members = editingDataType.value.members
  ;[members[idx + 1], members[idx]] = [members[idx], members[idx + 1]]
}

const onMemberTypeChange = () => {
  if (currentEditingMember.value && currentEditingMember.value.type !== 'sequence') {
    currentEditingMember.value.subType = undefined
  }
}

// ==================== 接口操作 ====================

const addInterface = (type: string) => {
  if (!editingModel.value) return
  const newInterface: ModelInterface = {
    name: '',
    dataType: dataTypes.value[0]?.name || 'int32',
    topic: needTopic(type) ? '' : undefined,
    displayName: '',
    description: ''
  }
  const map: Record<string, keyof Model> = {
    '参数接口': 'paramInterfaces',
    '输入接口': 'inputInterfaces',
    '输出接口': 'outputInterfaces',
    '初始化接口': 'initInterfaces',
    '事件接口': 'eventInterfaces'
  }
  const key = map[type]
  if (!editingModel.value[key]) {
    editingModel.value[key] = [] as any
  }
  ;(editingModel.value[key] as ModelInterface[]).push(newInterface)
}

const removeInterface = (type: string, idx: number) => {
  if (!editingModel.value) return
  const map: Record<string, keyof Model> = {
    '参数接口': 'paramInterfaces',
    '输入接口': 'inputInterfaces',
    '输出接口': 'outputInterfaces',
    '初始化接口': 'initInterfaces',
    '事件接口': 'eventInterfaces'
  }
  const key = map[type]
  if (editingModel.value[key]) {
    (editingModel.value[key] as ModelInterface[]).splice(idx, 1)
  }
}

const moveInterfaceUp = (type: string, idx: number) => {
  if (!editingModel.value || idx === 0) return
  const map: Record<string, keyof Model> = {
    '参数接口': 'paramInterfaces',
    '输入接口': 'inputInterfaces',
    '输出接口': 'outputInterfaces',
    '初始化接口': 'initInterfaces',
    '事件接口': 'eventInterfaces'
  }
  const key = map[type]
  const interfaces = editingModel.value[key] as ModelInterface[]
  if (interfaces) {
    ;[interfaces[idx - 1], interfaces[idx]] = [interfaces[idx], interfaces[idx - 1]]
  }
}

const moveInterfaceDown = (type: string, idx: number) => {
  if (!editingModel.value) return
  const map: Record<string, keyof Model> = {
    '参数接口': 'paramInterfaces',
    '输入接口': 'inputInterfaces',
    '输出接口': 'outputInterfaces',
    '初始化接口': 'initInterfaces',
    '事件接口': 'eventInterfaces'
  }
  const key = map[type]
  const interfaces = editingModel.value[key] as ModelInterface[]
  if (interfaces && idx < interfaces.length - 1) {
    ;[interfaces[idx + 1], interfaces[idx]] = [interfaces[idx], interfaces[idx + 1]]
  }
}

// ==================== XML 导入导出 ====================

const exportToXML = () => {
  dataTypes.value.forEach(dt => {
    if (!dt.uuid) dt.uuid = generateUUID()
  })
  models.value.forEach(model => {
    if (!model.uuid) model.uuid = generateUUID()
  })
  
  let xml = `<?xml version='1.0' encoding='UTF-8'?>\n`
  xml += `<root type="SERVICE">\n`
  xml += `  <types>\n`
  
  dataTypes.value.forEach(dataType => {
    xml += `    <type>\n`
    xml += `      <info alias="${dataType.name}" create_time="" creator="" last_modify="" modifier="" name="${dataType.name}" note="${dataType.description || dataType.name}" type="TYPE_STRUCT" uuid="${dataType.uuid}" version=""/>\n`
    
    dataType.members.forEach(member => {
      let memberDisplayType = member.type
      let memberElemType = '--'
      let memberSequenceType = ''
      let memberTypeRef = ''
      
      if (member.type === 'sequence') {
        memberDisplayType = 'sequence'
        if (member.subType) {
          if (baseTypes.includes(member.subType)) {
            memberElemType = member.subType
            memberSequenceType = member.subType
          } else {
            const subTypeUUID = getDataTypeUUID(member.subType)
            memberElemType = subTypeUUID
            memberSequenceType = subTypeUUID
          }
        } else {
          memberElemType = 'int32'
          memberSequenceType = 'int32'
        }
        memberTypeRef = 'sequence'
      } else if (baseTypes.includes(member.type)) {
        memberTypeRef = member.type
      } else {
        memberTypeRef = getDataTypeUUID(member.type)
      }
      
      xml += `      <member array="" display="${memberDisplayType}" display_name="${member.displayName || member.varName}" elem_type="${memberElemType}" name="${member.varName || 'field'}" note="${member.description || ''}" range="${member.range || ''}" sequence_type="${memberSequenceType}" type="${memberTypeRef}" unit_name="${member.unit || ''}"/>\n`
    })
    xml += `    </type>\n`
  })
  
  xml += `  </types>\n`
  xml += `  <models>\n`
  
  models.value.forEach(model => {
    xml += `    <model>\n`
    xml += `      <info address="" display_name="${model.displayName || model.name}" language="" name="${model.name}" note="" person="" software="" type="TYPE_MODEL" uuid="${model.uuid}" version=""/>\n`
    xml += `      <members>\n`
    model.paramInterfaces.forEach((param, idx) => {
      const typeRef = getInterfaceTypeRef(param.dataType)
      xml += `        <member display="${param.displayName || param.name}" name="${param.name || `param_${idx}`}" note="${param.description || ''}" type="${typeRef}"/>\n`
    })
    xml += `      </members>\n`
    xml += `      <inputs>\n`
    model.inputInterfaces.forEach((input, idx) => {
      const typeRef = getInterfaceTypeRef(input.dataType)
      xml += `        <input alias="${input.topic || input.name}" display="${input.displayName || input.name}" name="${input.name || `input_${idx}`}" note="${input.description || ''}" type="${typeRef}"/>\n`
    })
    xml += `      </inputs>\n`
    xml += `      <outputs>\n`
    model.outputInterfaces.forEach((output, idx) => {
      const typeRef = getInterfaceTypeRef(output.dataType)
      xml += `        <output alias="${output.topic || output.name}" display="${output.displayName || output.name}" name="${output.name || `output_${idx}`}" note="${output.description || ''}" type="${typeRef}"/>\n`
    })
    xml += `      </outputs>\n`
    xml += `      <threads>\n`
    model.initInterfaces.forEach((init, idx) => {
      xml += `        <thread interval="" name="${init.name || `init_${idx}`}" note="${init.description || ''}" sync_type="" timer_type=""/>\n`
    })
    model.eventInterfaces.forEach((event, idx) => {
      xml += `        <thread interval="" name="${event.name || `event_${idx}`}" note="${event.description || ''}" sync_type="" timer_type=""/>\n`
    })
    if (model.initInterfaces.length === 0 && model.eventInterfaces.length === 0) {
      xml += `        <thread interval="" name="run" note="" sync_type="" timer_type=""/>\n`
    }
    xml += `      </threads>\n`
    xml += `    </model>\n`
  })
  
  xml += `  </models>\n`
  xml += `  <service>\n`
  xml += `    <info name="${models.value[0]?.name || 'Service'}" type="TYPE_SERVICE" uuid="${models.value[0]?.name || 'Service'}"/>\n`
  xml += `    <members/>\n`
  xml += `    <models>\n`
  models.value.forEach(model => {
    xml += `      <model model="${model.uuid}"/>\n`
  })
  xml += `    </models>\n`
  xml += `  </service>\n`
  xml += `</root>`
  
  const blob = new Blob([xml], { type: 'application/xml' })
  const url = URL.createObjectURL(blob)
  const a = document.createElement('a')
  a.href = url
  a.download = `model-export-${new Date().toISOString().slice(0, 19)}.xml`
  document.body.appendChild(a)
  a.click()
  document.body.removeChild(a)
  URL.revokeObjectURL(url)
  alert('导出XML成功！')
}

const importFromXML = () => {
  const input = document.createElement('input')
  input.type = 'file'
  input.accept = '.xml'
  input.onchange = async (e: Event) => {
    const file = (e.target as HTMLInputElement).files?.[0]
    if (!file) return
    
    const reader = new FileReader()
    reader.onload = async (event) => {
      isLoading.value = true
      
      try {
        const content = event.target?.result as string
        const parser = new DOMParser()
        const xmlDoc = parser.parseFromString(content, 'text/xml')
        
        const newDataTypes: DataType[] = []
        const newModels: Model[] = []
        const uuidToTypeName: Map<string, string> = new Map()
        
        const typeElements = xmlDoc.getElementsByTagName('type')
        for (let i = 0; i < typeElements.length; i++) {
          const typeElem = typeElements[i]
          const infoElem = typeElem.getElementsByTagName('info')[0]
          const name = infoElem?.getAttribute('name') || ''
          const description = infoElem?.getAttribute('note') || ''
          const uuid = infoElem?.getAttribute('uuid') || generateUUID()
          
          uuidToTypeName.set(uuid, name)
          
          const members: DataTypeMember[] = []
          const memberElems = typeElem.getElementsByTagName('member')
          for (let j = 0; j < memberElems.length; j++) {
            const memberElem = memberElems[j]
            let type = memberElem.getAttribute('display') || memberElem.getAttribute('type') || 'int32'
            let subType: string | undefined = undefined
            
            if (type === 'sequence') {
              const elemType = memberElem.getAttribute('elem_type') || ''
              if (elemType && elemType !== '--') {
                subType = uuidToTypeName.has(elemType) ? uuidToTypeName.get(elemType) : elemType
              }
            } else {
              if (uuidToTypeName.has(type)) {
                type = uuidToTypeName.get(type)!
              }
            }
            
            members.push({
              displayName: memberElem.getAttribute('display_name') || '',
              type: type,
              subType: subType,
              range: memberElem.getAttribute('range') || '',
              varName: memberElem.getAttribute('name') || '',
              defaultValue: '',
              unit: memberElem.getAttribute('unit_name') || '',
              description: memberElem.getAttribute('note') || ''
            })
          }
          
          newDataTypes.push({ id: null, name, description, members, uuid })
        }
        
        const modelElements = xmlDoc.getElementsByTagName('model')
        for (let i = 0; i < modelElements.length; i++) {
          const modelElem = modelElements[i]
          const infoElem = modelElem.getElementsByTagName('info')[0]
          const name = infoElem?.getAttribute('name') || ''
          const displayName = infoElem?.getAttribute('display_name') || name
          const uuid = infoElem?.getAttribute('uuid') || generateUUID()
          
          const paramInterfaces: ModelInterface[] = []
          const inputInterfaces: ModelInterface[] = []
          const outputInterfaces: ModelInterface[] = []
          const initInterfaces: ModelInterface[] = []
          const eventInterfaces: ModelInterface[] = []
          
          const memberElems = modelElem.getElementsByTagName('members')
          if (memberElems.length > 0) {
            const members = memberElems[0].getElementsByTagName('member')
            for (let j = 0; j < members.length; j++) {
              const member = members[j]
              let dataType = member.getAttribute('type') || 'int32'
              if (uuidToTypeName.has(dataType)) dataType = uuidToTypeName.get(dataType)!
              paramInterfaces.push({
                name: member.getAttribute('name') || '',
                dataType: dataType,
                displayName: member.getAttribute('display') || '',
                description: member.getAttribute('note') || ''
              })
            }
          }
          
          const inputElems = modelElem.getElementsByTagName('inputs')
          if (inputElems.length > 0) {
            const inputs = inputElems[0].getElementsByTagName('input')
            for (let j = 0; j < inputs.length; j++) {
              const input = inputs[j]
              let dataType = input.getAttribute('type') || 'int32'
              if (uuidToTypeName.has(dataType)) dataType = uuidToTypeName.get(dataType)!
              inputInterfaces.push({
                name: input.getAttribute('name') || '',
                dataType: dataType,
                topic: input.getAttribute('alias') || '',
                displayName: input.getAttribute('display') || '',
                description: input.getAttribute('note') || ''
              })
            }
          }
          
          const outputElems = modelElem.getElementsByTagName('outputs')
          if (outputElems.length > 0) {
            const outputs = outputElems[0].getElementsByTagName('output')
            for (let j = 0; j < outputs.length; j++) {
              const output = outputs[j]
              let dataType = output.getAttribute('type') || 'int32'
              if (uuidToTypeName.has(dataType)) dataType = uuidToTypeName.get(dataType)!
              outputInterfaces.push({
                name: output.getAttribute('name') || '',
                dataType: dataType,
                topic: output.getAttribute('alias') || '',
                displayName: output.getAttribute('display') || '',
                description: output.getAttribute('note') || ''
              })
            }
          }
          
          const threadElems = modelElem.getElementsByTagName('threads')
          if (threadElems.length > 0) {
            const threads = threadElems[0].getElementsByTagName('thread')
            for (let j = 0; j < threads.length; j++) {
              const thread = threads[j]
              const threadName = thread.getAttribute('name') || ''
              if (threadName.startsWith('init')) {
                initInterfaces.push({
                  name: threadName,
                  dataType: 'void',
                  displayName: thread.getAttribute('note') || '',
                  description: thread.getAttribute('note') || ''
                })
              } else {
                eventInterfaces.push({
                  name: threadName,
                  dataType: 'void',
                  displayName: thread.getAttribute('note') || '',
                  description: thread.getAttribute('note') || ''
                })
              }
            }
          }
          
          if (initInterfaces.length === 0 && eventInterfaces.length === 0) {
            eventInterfaces.push({
              name: 'run',
              dataType: 'void',
              displayName: '运行',
              description: '默认运行线程'
            })
          }
          
          newModels.push({ id: null, name, displayName, paramInterfaces, inputInterfaces, outputInterfaces, initInterfaces, eventInterfaces, uuid })
        }
        
        dataTypes.value = newDataTypes
        models.value = newModels
        await saveProjectData()  // 保存到当前工程
        
        selectedDataType.value = null
        selectedModel.value = null
        editingDataType.value = null
        editingModel.value = null
        
        alert(`导入成功！\n数据结构: ${newDataTypes.length} 个\n模型: ${newModels.length} 个`)
      } catch (err) {
        console.error('导入失败:', err)
        alert('导入失败：文件格式错误')
      } finally {
        isLoading.value = false
      }
    }
    reader.readAsText(file)
  }
  input.click()
}

// ==================== 代码生成 ====================
// 修改 generateCode 函数
const generateProgress = ref('')

const generateCode = async () => {
    if (!selectedModelForGenerate.value) {
        alert('请选择模型')
        return
    }
    
    isGenerating.value = true
    
    generateProgress.value = '正在准备XML文件...'
    try {
        const model = models.value.find(m => m.name === selectedModelForGenerate.value)
        if (!model) throw new Error('模型不存在')
        
        const relatedTypes = getRelatedTypes(selectedModelForGenerate.value)
        relatedTypes.forEach(dt => { if (!dt.uuid) dt.uuid = generateUUID() })
        if (!model.uuid) model.uuid = generateUUID()
        
        let xml = `<?xml version='1.0' encoding='UTF-8'?>\n`
        xml += `<root type="SERVICE">\n`
        xml += `  <types>\n`
        
        relatedTypes.forEach(dataType => {
            xml += `    <type>\n`
            xml += `      <info alias="${dataType.name}" create_time="" creator="" last_modify="" modifier="" name="${dataType.name}" note="${dataType.description || dataType.name}" type="TYPE_STRUCT" uuid="${dataType.uuid}" version=""/>\n`
            
            dataType.members.forEach(member => {
                let memberDisplayType = member.type
                let memberElemType = '--'
                let memberSequenceType = ''
                
                if (member.type === 'sequence') {
                    memberDisplayType = 'sequence'
                    if (member.subType) {
                        if (baseTypes.includes(member.subType)) {
                            memberElemType = member.subType
                            memberSequenceType = member.subType
                        } else {
                            const subTypeUUID = getDataTypeUUID(member.subType)
                            memberElemType = subTypeUUID
                            memberSequenceType = subTypeUUID
                        }
                    } else {
                        memberElemType = 'int32'
                        memberSequenceType = 'int32'
                    }
                }
                
                const memberTypeRef = member.type === 'sequence' ? 'sequence' : (baseTypes.includes(member.type) ? member.type : getDataTypeUUID(member.type))
                
                xml += `      <member array="" display="${memberDisplayType}" display_name="${member.displayName || member.varName}" elem_type="${memberElemType}" name="${member.varName || 'field'}" note="${member.description || ''}" range="${member.range || ''}" sequence_type="${memberSequenceType}" type="${memberTypeRef}" unit_name="${member.unit || ''}"/>\n`
            })
            xml += `    </type>\n`
        })
        
        xml += `  </types>\n`
        xml += `  <models>\n`
        xml += `    <model>\n`
        xml += `      <info address="" display_name="${model.displayName || model.name}" language="${selectedLanguage.value}" name="${model.name}" note="" person="" software="" type="TYPE_MODEL" uuid="${model.uuid}" version=""/>\n`
        xml += `      <members>\n`
        model.paramInterfaces.forEach((param, idx) => {
            xml += `        <member display="${param.displayName || param.name}" name="${param.name || `param_${idx}`}" note="${param.description || ''}" type="${getInterfaceTypeRef(param.dataType)}"/>\n`
        })
        xml += `      </members>\n`
        xml += `      <inputs>\n`
        model.inputInterfaces.forEach((input, idx) => {
            xml += `        <input alias="${input.topic || input.name}" display="${input.displayName || input.name}" name="${input.name || `input_${idx}`}" note="${input.description || ''}" type="${getInterfaceTypeRef(input.dataType)}"/>\n`
        })
        xml += `      </inputs>\n`
        xml += `      <outputs>\n`
        model.outputInterfaces.forEach((output, idx) => {
            xml += `        <output alias="${output.topic || output.name}" display="${output.displayName || output.name}" name="${output.name || `output_${idx}`}" note="${output.description || ''}" type="${getInterfaceTypeRef(output.dataType)}"/>\n`
        })
        xml += `      </outputs>\n`
        xml += `      <threads>\n`
        model.initInterfaces.forEach((init, idx) => {
            xml += `        <thread interval="" name="${init.name || `init_${idx}`}" note="${init.description || ''}" sync_type="" timer_type=""/>\n`
        })
        model.eventInterfaces.forEach((event, idx) => {
            xml += `        <thread interval="" name="${event.name || `event_${idx}`}" note="${event.description || ''}" sync_type="" timer_type=""/>\n`
        })
        if (model.initInterfaces.length === 0 && model.eventInterfaces.length === 0) {
            xml += `        <thread interval="" name="run" note="" sync_type="" timer_type=""/>\n`
        }
        xml += `      </threads>\n`
        xml += `    </model>\n`
        xml += `  </models>\n`
        xml += `  <service>\n`
        xml += `    <info name="${model.name}" type="TYPE_SERVICE" uuid="${model.name}"/>\n`
        xml += `    <members/>\n`
        xml += `    <models>\n`
        xml += `      <model model="${model.uuid}"/>\n`
        xml += `    </models>\n`
        xml += `  </service>\n`
        xml += `</root>`
        
          generateProgress.value = '正在发送请求到服务器...'
        // 发送请求并处理ZIP文件下载
        const response = await fetch('http://192.168.156.20:8080/generate', {
            method: 'POST',
            headers: { 'Content-Type': 'application/xml' },
            body: xml
        })
        
        if (response.ok) {
           generateProgress.value = '正在下载代码包...'
            // 获取文件名
            const contentDisposition = response.headers.get('Content-Disposition')
            let filename = `${model.name}_code.zip`
            if (contentDisposition) {
                const match = contentDisposition.match(/filename[^;=\n]*=((['"]).*?\2|[^;\n]*)/)
                if (match && match[1]) {
                    filename = match[1].replace(/['"]/g, '')
                }
            }
            
            // 下载ZIP文件
            const blob = await response.blob()
            const url = URL.createObjectURL(blob)
            const a = document.createElement('a')
            a.href = url
            a.download = filename
            document.body.appendChild(a)
            a.click()
            document.body.removeChild(a)
            URL.revokeObjectURL(url)
            
            alert('代码生成成功！')
            showGenerateDialog.value = false
            selectedModelForGenerate.value = ''
            selectedLanguage.value = 'python'
        } else {
            const errorText = await response.text()
            let errorMsg = '代码生成失败'
            try {
                const errorJson = JSON.parse(errorText)
                errorMsg = errorJson.error || errorMsg
            } catch {
                errorMsg = errorText || errorMsg
            }
            alert(`代码生成失败: ${errorMsg}`)
        }
    } catch (error) {
        console.error('请求失败:', error)
        alert('连接服务器失败，请检查网络连接')
    } finally {
        isGenerating.value = false
         generateProgress.value = ''
    }
}

// ==================== 初始化 ====================

const initializeData = async () => {
  isLoading.value = true
  
   try {
        // 从服务器加载当前工程的数据
        await Promise.all([
            fetchDataTypesFromServer(),
            fetchModelsFromServer()
        ])
    } catch (error) {
        console.error('Failed to initialize data:', error)
    } finally {
        isLoading.value = false
    }
}

const initSampleData = () => {
  const vector3: DataType = {
    id: nextDataTypeId++,
    name: 'Vector3',
    description: '三维向量',
    uuid: generateUUID(),
    members: [
      { displayName: 'X坐标', type: 'real32', range: '-inf~inf', varName: 'x', defaultValue: '0', unit: 'm', description: 'X轴分量' },
      { displayName: 'Y坐标', type: 'real32', range: '-inf~inf', varName: 'y', defaultValue: '0', unit: 'm', description: 'Y轴分量' },
      { displayName: 'Z坐标', type: 'real32', range: '-inf~inf', varName: 'z', defaultValue: '0', unit: 'm', description: 'Z轴分量' }
    ]
  }
  const pose: DataType = {
    id: nextDataTypeId++,
    name: 'Pose',
    description: '位姿（位置+方向）',
    uuid: generateUUID(),
    members: [
      { displayName: '位置', type: 'Vector3', range: '', varName: 'position', defaultValue: '', unit: 'm', description: '位置坐标' },
      { displayName: '朝向四元数', type: 'string', range: '', varName: 'orientation', defaultValue: '0,0,0,1', unit: '', description: '四元数' },
      { displayName: '点云数据', type: 'sequence', subType: 'real32', range: '', varName: 'pointCloud', defaultValue: '', unit: '', description: '点云数据' }
    ]
  }
  dataTypes.value.push(vector3, pose)

  const robotModel: Model = {
    id: nextModelId++,
    name: 'RobotController',
    displayName: '机器人控制器',
    uuid: generateUUID(),
    paramInterfaces: [
      { name: 'max_speed', dataType: 'real32', displayName: '最大速度', description: '线速度上限' }
    ],
    inputInterfaces: [
      { name: 'cmd_vel', dataType: 'Vector3', topic: '/cmd_vel', displayName: '速度指令', description: '线速度与角速度' }
    ],
    outputInterfaces: [
      { name: 'odom', dataType: 'Pose', topic: '/odom', displayName: '里程计数据', description: '当前位姿' }
    ],
    initInterfaces: [
      { name: 'init', dataType: 'void', displayName: '初始化', description: '模型初始化' }
    ],
    eventInterfaces: [
      { name: 'run', dataType: 'void', displayName: '运行', description: '主运行循环' },
      { name: 'battery_low', dataType: 'bool', displayName: '低电量事件', description: '电量低于阈值触发' }
    ]
  }
  models.value.push(robotModel)
}

// 在顶部添加
const currentProject = ref<any>(null)

// 加载工程数据
const loadProjectData = async () => {
    const projectStr = sessionStorage.getItem('currentProject')
    if (!projectStr) {
        router.replace('/projects')
        return
    }

    currentProject.value = JSON.parse(projectStr)
    
    // 不从这里加载数据，让 initializeData 从服务器加载
    // 初始化数据（从服务器按工程ID加载）
    await initializeData()
    
    // 如果服务器没有数据，才使用 sessionStorage 中的数据作为备选
    if (dataTypes.value.length === 0 && models.value.length === 0 && currentProject.value?.data) {
        try {
            const projectData = currentProject.value.data
            if (projectData.dataTypes) {
                dataTypes.value = projectData.dataTypes
            }
            if (projectData.models) {
                models.value = projectData.models
            }
        } catch (e) {
            console.error('Failed to load project data from session:', e)
        }
    }
}

// 保存工程数据
const saveProjectData = async () => {
    if (!currentProject.value) return
    
    const projectData = {
        dataTypes: dataTypes.value,
        models: models.value
    }
    
    try {
        const response = await fetch(`${API_BASE_URL}/projects/${currentProject.value.id}/save`, {
            method: 'POST',
            headers: getHeaders(),
            body: JSON.stringify(projectData)
        })
        
        if (response.ok) {
            console.log('Project saved successfully')
        } else {
            console.error('Failed to save project')
        }
    } catch (error) {
        console.error('Failed to save project:', error)
    }
}

// 在初始化时加载工程数据
onMounted(async () => {
    // 加载工程数据（内部会调用 initializeData 从服务器加载）
    await loadProjectData()
    setupWebSocket()
})


onUnmounted(() => {
  if (ws) {
    ws.close()
  }
})
</script>


<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.model-designer {
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

.header-right {
  display: flex;
  align-items: center;
  gap: 16px;
}

.import-export-group {
  display: flex;
  gap: 8px;
}

.btn-import, .btn-export {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 8px 16px;
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  color: #cbd5e1;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-import:hover, .btn-export:hover {
  background: rgba(59, 130, 246, 0.2);
  border-color: rgba(59, 130, 246, 0.4);
  color: #3b82f6;
}

.btn-generate {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 8px 16px;
  background: linear-gradient(135deg, #f59e0b, #ef4444);
  border: none;
  border-radius: 40px;
  color: white;
  font-size: 13px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-generate:hover {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(245, 158, 11, 0.4);
}

.stats-badge {
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

.stats-dot.active {
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
  padding: 8px;
  flex-shrink: 0;
}

.tabs {
  display: flex;
  gap: 8px;
  background: rgba(0, 0, 0, 0.3);
  padding: 6px;
  border-radius: 60px;
}

.tab {
  flex: 1;
  padding: 10px 20px;
  background: transparent;
  border: none;
  border-radius: 40px;
  font-size: 14px;
  font-weight: 500;
  color: #94a3b8;
  cursor: pointer;
  transition: all 0.2s;
}

.tab.active {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
  box-shadow: 0 2px 8px rgba(59, 130, 246, 0.3);
}

.list-container {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  padding: 16px;
}

.list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16px;
  padding: 0 8px;
}

.list-title {
  font-size: 14px;
  font-weight: 600;
  color: #94a3b8;
}

.btn-add {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  padding: 6px 14px;
  border-radius: 20px;
  color: white;
  font-size: 12px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-add:hover {
  transform: scale(1.02);
  box-shadow: 0 2px 8px rgba(59, 130, 246, 0.4);
}

.list-items {
  flex: 1;
  overflow-y: auto;
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.list-item {
  display: flex;
  align-items: center;
  gap: 14px;
  padding: 12px 16px;
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.25s ease;
  background: rgba(255, 255, 255, 0.02);
  border: 1px solid transparent;
}

.list-item:hover {
  background: rgba(59, 130, 246, 0.1);
  border-color: rgba(59, 130, 246, 0.3);
  transform: translateX(4px);
}

.list-item.active {
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
}

.icon-bg.data {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.icon-bg.model {
  background: rgba(139, 92, 246, 0.2);
  color: #8b5cf6;
}

.item-info {
  flex: 1;
}

.item-name {
  font-weight: 600;
  color: white;
  font-size: 14px;
  margin-bottom: 4px;
}

.item-desc {
  font-size: 11px;
  color: #94a3b8;
}

.item-actions {
  display: flex;
  gap: 6px;
  opacity: 0;
  transition: opacity 0.2s;
}

.list-item:hover .item-actions {
  opacity: 1;
}

.btn-icon {
  width: 28px;
  height: 28px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
}

.btn-icon.edit {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.btn-icon.edit:hover {
  background: rgba(59, 130, 246, 0.4);
}

.btn-icon.delete {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.btn-icon.delete:hover {
  background: rgba(239, 68, 68, 0.4);
}

.right-content {
  flex: 1;
  min-width: 0;
  overflow-y: auto;
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

.type-badge {
  display: flex;
  align-items: center;
  gap: 10px;
  background: rgba(255, 255, 255, 0.05);
  padding: 8px 20px;
  border-radius: 40px;
}

.type-badge.data svg {
  color: #10b981;
}

.type-badge.model svg {
  color: #8b5cf6;
}

.type-badge h2 {
  font-size: 18px;
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
  font-size: 12px;
  font-weight: 500;
  background: rgba(16, 185, 129, 0.15);
  color: #10b981;
  border: 1px solid rgba(16, 185, 129, 0.3);
}

.form-group {
  margin-bottom: 20px;
}

.form-group label {
  display: block;
  font-size: 12px;
  font-weight: 500;
  color: #94a3b8;
  margin-bottom: 8px;
}

.form-input, .form-textarea, .form-select {
  width: 100%;
  padding: 12px 16px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 16px;
  color: white;
  font-size: 14px;
  transition: all 0.2s;
}

.form-input:focus, .form-textarea:focus, .form-select:focus {
  outline: none;
  border-color: #3b82f6;
  box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.2);
}

.form-textarea {
  resize: vertical;
}

.members-section, .interfaces-section {
  margin-top: 28px;
}

.section-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.section-title {
  display: flex;
  align-items: center;
  gap: 12px;
  font-size: 14px;
  font-weight: 600;
  color: #cbd5e1;
}

.section-count {
  font-size: 11px;
  color: #64748b;
  background: rgba(0, 0, 0, 0.3);
  padding: 2px 8px;
  border-radius: 20px;
}

.btn-secondary {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  padding: 8px 16px;
  border-radius: 12px;
  color: #cbd5e1;
  font-size: 12px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-secondary:hover {
  background: rgba(255, 255, 255, 0.15);
}

.btn-primary {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  padding: 12px 24px;
  border-radius: 40px;
  color: white;
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-primary:hover {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
}

.members-table-wrapper,
.interfaces-table-wrapper {
  overflow-x: auto;
  border-radius: 16px;
  background: rgba(0, 0, 0, 0.2);
}

.members-table,
.interfaces-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}

.members-table th,
.interfaces-table th {
  text-align: left;
  padding: 12px 12px;
  background: rgba(0, 0, 0, 0.3);
  color: #94a3b8;
  font-weight: 500;
  font-size: 12px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}

.members-table td,
.interfaces-table td {
  padding: 10px 12px;
  color: #e2e8f0;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
  vertical-align: middle;
}

.members-table tr:hover td,
.interfaces-table tr:hover td {
  background: rgba(59, 130, 246, 0.05);
}

.index-cell {
  color: #64748b;
  font-size: 12px;
  text-align: center;
}

.type-tag {
  display: inline-block;
  padding: 2px 8px;
  background: rgba(59, 130, 246, 0.2);
  border-radius: 12px;
  font-size: 11px;
  color: #3b82f6;
}

.type-tag.type-sequence {
  background: linear-gradient(135deg, rgba(139, 92, 246, 0.2), rgba(59, 130, 246, 0.2));
  color: #a78bfa;
  cursor: help;
}

.desc-cell {
  max-width: 150px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.table-input,
.table-select {
  width: 100%;
  padding: 6px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  color: white;
  font-size: 12px;
}

.table-input:focus,
.table-select:focus {
  outline: none;
  border-color: #3b82f6;
}

.table-actions {
  display: flex;
  gap: 4px;
}

.table-btn {
  padding: 4px 8px;
  border-radius: 6px;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
  background: rgba(255, 255, 255, 0.08);
  color: #94a3b8;
}

.table-btn:hover:not(:disabled) {
  background: rgba(59, 130, 246, 0.3);
  color: #3b82f6;
}

.table-btn.move-up:hover:not(:disabled),
.table-btn.move-down:hover:not(:disabled) {
  background: rgba(59, 130, 246, 0.3);
  color: #3b82f6;
}

.table-btn.edit:hover:not(:disabled) {
  background: rgba(16, 185, 129, 0.3);
  color: #10b981;
}

.table-btn.delete:hover:not(:disabled) {
  background: rgba(239, 68, 68, 0.3);
  color: #ef4444;
}

.table-btn:disabled {
  opacity: 0.3;
  cursor: not-allowed;
}

.empty-table {
  text-align: center;
  padding: 40px !important;
  color: #64748b;
}

.editor-actions {
  display: flex;
  gap: 16px;
  margin-top: 28px;
  justify-content: flex-end;
}

.interface-tabs {
  display: flex;
  gap: 8px;
  margin: 20px 0;
  flex-wrap: wrap;
}

.interface-tab {
  padding: 8px 16px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  color: #94a3b8;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
}

.interface-tab.active {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
  border-color: transparent;
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

.empty-list {
  text-align: center;
  padding: 60px 20px;
  color: #64748b;
}

.empty-illustration svg {
  color: #334155;
  margin-bottom: 16px;
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.7);
  backdrop-filter: blur(8px);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}

.modal-content {
  width: 550px;
  max-width: 90%;
  max-height: 85vh;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.modal-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 20px 24px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.modal-header h3 {
  color: white;
  font-size: 18px;
  font-weight: 600;
}

.modal-close {
  width: 32px;
  height: 32px;
  border-radius: 10px;
  background: rgba(255, 255, 255, 0.1);
  border: none;
  color: white;
  font-size: 20px;
  cursor: pointer;
}

.modal-body {
  flex: 1;
  overflow-y: auto;
  padding: 24px;
}

.modal-actions {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  padding: 16px 24px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
}

.subtype-cell {
  min-width: 180px;
}

.subtype-preview {
  display: block;
  font-size: 11px;
  color: #64748b;
  margin-bottom: 4px;
  font-family: monospace;
}

.subtype-select {
  width: 100%;
  padding: 6px 8px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  color: white;
  font-size: 12px;
}

.subtype-select:focus {
  outline: none;
  border-color: #8b5cf6;
}

.subtype-placeholder {
  color: #64748b;
  font-size: 12px;
}

.subtype-hint {
  margin-top: 8px;
  padding: 8px 12px;
  background: rgba(139, 92, 246, 0.15);
  border-radius: 8px;
  font-size: 12px;
}

.subtype-hint code {
  color: #a78bfa;
  background: rgba(0, 0, 0, 0.3);
  padding: 2px 6px;
  border-radius: 4px;
  font-family: monospace;
}

.generate-preview {
  margin-top: 16px;
  padding: 12px;
  background: rgba(59, 130, 246, 0.1);
  border-radius: 12px;
  border: 1px solid rgba(59, 130, 246, 0.2);
}

.preview-title {
  font-size: 14px;
  font-weight: 500;
  color: #3b82f6;
  margin-bottom: 8px;
}

.preview-info {
  font-size: 12px;
  color: #94a3b8;
}

.loading-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.8);
  backdrop-filter: blur(8px);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 2000;
}

.loading-spinner {
  text-align: center;
  background: rgba(15, 25, 35, 0.9);
  padding: 32px 48px;
  border-radius: 24px;
  border: 1px solid rgba(59, 130, 246, 0.3);
}

.spinner {
  width: 48px;
  height: 48px;
  border: 3px solid rgba(59, 130, 246, 0.3);
  border-top-color: #3b82f6;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin: 0 auto 16px;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

.loading-spinner p {
  color: #cbd5e1;
  font-size: 14px;
}

::-webkit-scrollbar {
  width: 6px;
  height: 6px;
}

::-webkit-scrollbar-track {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 4px;
}

::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

@media (max-width: 1024px) {
  .left-sidebar {
    width: 280px;
  }
  
  .members-table th,
  .members-table td {
    padding: 8px;
  }
}

@media (max-width: 768px) {
  .main-layout {
    flex-direction: column;
  }
  
  .left-sidebar {
    width: 100%;
    max-height: 320px;
  }
  
  .detail-card {
    padding: 20px;
  }
  
  .members-table-wrapper,
  .interfaces-table-wrapper {
    overflow-x: auto;
  }
  
  .members-table,
  .interfaces-table {
    min-width: 900px;
  }
  
  .header-right {
    flex-direction: column;
    align-items: flex-end;
    gap: 8px;
  }
}
</style>