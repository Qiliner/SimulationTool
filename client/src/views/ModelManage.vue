<template>
  <div class="model-manage-container">
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 顶部导航栏 -->
    <div class="navbar glass-panel">
      <div class="nav-left">
        <div class="back-icon" @click="goBack">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M19 12H5M12 19l-7-7 7-7"/>
          </svg>
        </div>
        <div class="logo-icon">
          <svg width="32" height="32" viewBox="0 0 24 24" fill="none">
            <path d="M12 2L2 7L12 12L22 7L12 2Z" stroke="currentColor" stroke-width="1.5"/>
            <path d="M2 17L12 22L22 17" stroke="currentColor" stroke-width="1.5"/>
            <path d="M2 12L12 17L22 12" stroke="currentColor" stroke-width="1.5"/>
          </svg>
          <span class="logo-text">ZRSIM</span>
        </div>
        <span class="nav-title">模型与软件管理</span>
      </div>
      <div class="nav-right">
        <div class="user-info" v-if="userStore.userInfo">
          <span class="user-name">{{ userStore.userInfo.username }}</span>
          <span class="user-role" :class="roleClass">{{ userStore.userInfo.roleName }}</span>
        </div>
        <button class="btn-logout" @click="handleLogout">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/>
            <polyline points="16 17 21 12 16 7"/>
            <line x1="21" y1="12" x2="9" y2="12"/>
          </svg>
          退出
        </button>
      </div>
    </div>

    <!-- 主标签页 -->
    <div class="main-tabs glass-panel">
      <button :class="['tab', { active: activeMainTab === 'model' }]" @click="activeMainTab = 'model'">
        📦 模型管理
      </button>
      <button :class="['tab', { active: activeMainTab === 'software' }]" @click="activeMainTab = 'software'">
        💿 软件管理
      </button>
    </div>

    <!-- 模型管理区域 -->
    <div v-if="activeMainTab === 'model'" class="model-manage-area">
      <!-- 三库切换 -->
      <div class="lib-tabs">
        <button 
          v-for="lib in modelLibs" 
          :key="lib.id"
          :class="['lib-tab', { active: currentLibId === lib.id }]"
          @click="switchLib(lib.id)"
        >
          {{ lib.name }}
        </button>
      </div>

      <!-- 目录树和文件列表 -->
      <div class="content-layout">
       <div class="directory-tree glass-panel">
  <div class="tree-header">
    <span>📁 目录结构</span>
    <button class="btn-icon-small" @click="createDirectory" title="新建目录">
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <path d="M12 5v14M5 12h14"/>
      </svg>
    </button>
  </div>
  <div class="tree-content">
    <!-- 根目录 -->
    <div 
      class="directory-item root-directory"
      :class="{ active: selectedDirId === 0 }"
      @click="selectDirectory(0)"
    >
      <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <path d="M3 7v10a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2h-6l-2-2H5a2 2 0 0 0-2 2z"/>
      </svg>
      <span class="dir-name">根目录</span>
      <span class="dir-count" :class="{ 'has-count': rootFileCount > 0 }">
        {{ rootFileCount }}
      </span>
    </div>
    
    <!-- 子目录树 -->
    <div v-if="rootDirectories.length > 0">
      <DirectoryTree
        v-for="dir in rootDirectories"
        :key="dir.id"
        :directory="dir"
        :level="0"
        :all-directories="allDirectories"
        :file-count-map="fileCountMap"
        :selected-dir-id="selectedDirId"
        @select="selectDirectory"
        @delete="deleteDirectory"
        @rename="renameDirectory"
      />
    </div>
    <div v-if="rootDirectories.length === 0 && allDirectories.length === 0" class="empty-tree">
      暂无目录，点击"+"创建
    </div>
  </div>
</div>
        <!-- 右侧文件列表 -->
        <div class="file-list glass-panel">
          <div class="file-header">
            <span>📄 模型文件</span>
            <button class="btn-upload" @click="openUploadDialog">
              <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                <path d="M12 3v12m0 0-3-3m3 3 3-3M5 17v2a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-2"/>
              </svg>
              上传模型
            </button>
          </div>
          <div class="file-table-wrapper">
            <table class="file-table">
              <thead>
                <tr>
                  <th>模型名称</th>
                  <th>版本</th>
                  <th>文件大小</th>
                  <th>上传者</th>
                  <th>上传时间</th>
                  <th>操作</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="file in currentFiles" :key="file.id">
                  <td>{{ file.name }}</td>
                  <td>{{ file.version || '-' }}</td>
                  <td>{{ formatFileSize(file.fileSize) }}</td>
                  <td>{{ file.uploaderName }}</td>
                  <td>{{ formatDate(file.createdAt) }}</td>
                  <td>
                    <div class="file-actions">
                      <button class="btn-download" @click="downloadModel(file)" title="下载">↓</button>
                      <button class="btn-migrate" @click="openMigrateDialog(file)" title="迁移">↗</button>
                      <button class="btn-info" @click="showModelInfo(file)" title="详情">ℹ</button>
                      <button class="btn-delete" @click="deleteModelFile(file)" title="删除">×</button>
                    </div>
                  </td>
                </tr>
                <tr v-if="currentFiles.length === 0">
                  <td colspan="6" class="empty-table">暂无模型文件</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>

    <!-- 软件管理区域 -->
    <div v-if="activeMainTab === 'software'" class="software-manage-area">
      <div class="software-header">
        <span>💿 常用软件列表</span>
        <button class="btn-upload" @click="openSoftwareUploadDialog">
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M12 3v12m0 0-3-3m3 3 3-3M5 17v2a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2v-2"/>
          </svg>
          上传软件
        </button>
      </div>
      <div class="software-table-wrapper">
        <table class="software-table">
          <thead>
            <tr>
              <th>软件名称</th>
              <th>版本</th>
              <th>文件大小</th>
              <th>上传者</th>
              <th>上传时间</th>
              <th>操作</th>
            </tr>
          </thead>
          <tbody>
            <tr v-for="sw in softwareList" :key="sw.id">
              <td>{{ sw.name }}</td>
              <td>{{ sw.version || '-' }}</td>
              <td>{{ formatFileSize(sw.fileSize) }}</td>
              <td>{{ sw.uploaderName }}</td>
              <td>{{ formatDate(sw.createdAt) }}</td>
              <td>
                <div class="file-actions">
                  <button class="btn-download" @click="downloadSoftware(sw)" title="下载">↓</button>
                  <button class="btn-info" @click="showSoftwareInfo(sw)" title="详情">ℹ</button>
                  <button class="btn-delete" @click="deleteSoftware(sw)" title="删除">×</button>
                </div>
              </td>
            </tr>
            <tr v-if="softwareList.length === 0">
              <td colspan="6" class="empty-table">暂无软件</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>

    <!-- 上传模型弹窗 -->
    <div v-if="showUploadDialog" class="modal-overlay" @click.self="closeUploadDialog">
      <div class="modal-content glass-panel" style="width: 550px;">
        <div class="modal-header">
          <h3>📤 上传模型</h3>
          <button class="modal-close" @click="closeUploadDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>选择文件 *</label>
            <input type="file" ref="fileInput" @change="onFileSelected" class="form-input" accept=".zip,.tar.gz,.tar,.7z,.exe,.bin,.sh"/>
          </div>
          <div class="form-group">
            <label>目标库 *</label>
            <select v-model="uploadModel.libId" @change="onLibChange" class="form-select">
              <option v-for="lib in modelLibs" :key="lib.id" :value="lib.id">
                {{ lib.name }}
              </option>
            </select>
          </div>
          <div class="form-group">
            <label>目标目录</label>
            <select v-model="uploadModel.directoryId" class="form-select">
              
              <option v-for="dir in uploadDirectories" :key="dir.id" :value="dir.id">
                {{ dir.path || dir.name }}
              </option>
            </select>
          </div>
          <div class="form-group">
            <label>模型名称 *</label>
            <input v-model="uploadModel.name" class="form-input" placeholder="请输入模型名称" />
          </div>
          <div class="form-group">
            <label>版本</label>
            <input v-model="uploadModel.version" class="form-input" placeholder="例如: v1.0.0" />
          </div>
          <div class="form-group">
            <label>启动指令</label>
            <input v-model="uploadModel.startCmd" class="form-input" placeholder="例如: ./start.sh" />
          </div>
          <div class="form-group">
            <label>关闭指令</label>
            <input v-model="uploadModel.stopCmd" class="form-input" placeholder="例如: ./stop.sh" />
          </div>
          <div class="form-group">
            <label>描述</label>
            <textarea v-model="uploadModel.description" class="form-textarea" rows="3" placeholder="请输入模型描述"></textarea>
          </div>
          <div class="form-group" v-if="selectedFile">
            <label>已选文件</label>
            <div class="selected-file">{{ selectedFile.name }} ({{ formatFileSize(selectedFile.size) }})</div>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="uploadModelFile" :disabled="isUploading">
            {{ isUploading ? '上传中...' : '上传' }}
          </button>
          <button class="btn-secondary" @click="closeUploadDialog">取消</button>
        </div>
      </div>
    </div>

    <!-- 模型详情弹窗 -->
    <div v-if="showModelInfoDialog" class="modal-overlay" @click.self="closeModelInfoDialog">
      <div class="modal-content glass-panel" style="width: 500px;">
        <div class="modal-header">
          <h3>📄 模型详情</h3>
          <button class="modal-close" @click="closeModelInfoDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="info-item">
            <label>模型名称：</label>
            <span>{{ currentModelInfo.name }}</span>
          </div>
          <div class="info-item">
            <label>版本：</label>
            <span>{{ currentModelInfo.version || '-' }}</span>
          </div>
          <div class="info-item">
            <label>文件名：</label>
            <span>{{ currentModelInfo.fileName }}</span>
          </div>
          <div class="info-item">
            <label>文件大小：</label>
            <span>{{ formatFileSize(currentModelInfo.fileSize) }}</span>
          </div>
          <div class="info-item">
            <label>启动指令：</label>
            <code>{{ currentModelInfo.startCmd || '-' }}</code>
          </div>
          <div class="info-item">
            <label>关闭指令：</label>
            <code>{{ currentModelInfo.stopCmd || '-' }}</code>
          </div>
          <div class="info-item">
            <label>描述：</label>
            <p>{{ currentModelInfo.description || '-' }}</p>
          </div>
          <div class="info-item">
            <label>上传者：</label>
            <span>{{ currentModelInfo.uploaderName }}</span>
          </div>
          <div class="info-item">
            <label>上传时间：</label>
            <span>{{ formatDate(currentModelInfo.createdAt) }}</span>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-secondary" @click="closeModelInfoDialog">关闭</button>
        </div>
      </div>
    </div>

    <!-- 迁移模型弹窗 -->
    <div v-if="showMigrateDialog" class="modal-overlay" @click.self="closeMigrateDialog">
      <div class="modal-content glass-panel" style="width: 450px;">
        <div class="modal-header">
          <h3>↗ 迁移模型</h3>
          <button class="modal-close" @click="closeMigrateDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>目标库</label>
            <select v-model="migrateTarget.libId" class="form-select">
              <option v-for="lib in modelLibs" :key="lib.id" :value="lib.id" :disabled="lib.id === currentLibId">
                {{ lib.name }}
              </option>
            </select>
          </div>
          <div class="form-group">
            <label>目标目录</label>
            <select v-model="migrateTarget.dirId" class="form-select">
              <option value="0">根目录</option>
              <option v-for="dir in migrateDirectories" :key="dir.id" :value="dir.id">
                {{ dir.path || dir.name }}
              </option>
            </select>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="confirmMigrate" :disabled="isMigrating">
            {{ isMigrating ? '迁移中...' : '确认迁移' }}
          </button>
          <button class="btn-secondary" @click="closeMigrateDialog">取消</button>
        </div>
      </div>
    </div>

    <!-- 上传软件弹窗 -->
    <div v-if="showSoftwareUploadDialog" class="modal-overlay" @click.self="closeSoftwareUploadDialog">
      <div class="modal-content glass-panel" style="width: 550px;">
        <div class="modal-header">
          <h3>💿 上传软件</h3>
          <button class="modal-close" @click="closeSoftwareUploadDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>选择文件 *</label>
            <input type="file" ref="softwareFileInput" @change="onSoftwareFileSelected" class="form-input" />
          </div>
          <div class="form-group">
            <label>软件名称 *</label>
            <input v-model="uploadSoftware.name" class="form-input" placeholder="请输入软件名称" />
          </div>
          <div class="form-group">
            <label>版本</label>
            <input v-model="uploadSoftware.version" class="form-input" placeholder="例如: v1.0.0" />
          </div>
          <div class="form-group">
            <label>安装指令</label>
            <input v-model="uploadSoftware.installCmd" class="form-input" placeholder="例如: ./install.sh" />
          </div>
          <div class="form-group">
            <label>卸载指令</label>
            <input v-model="uploadSoftware.uninstallCmd" class="form-input" placeholder="例如: ./uninstall.sh" />
          </div>
          <div class="form-group">
            <label>启动指令</label>
            <input v-model="uploadSoftware.startCmd" class="form-input" placeholder="例如: ./start.sh" />
          </div>
          <div class="form-group">
            <label>关闭指令</label>
            <input v-model="uploadSoftware.stopCmd" class="form-input" placeholder="例如: ./stop.sh" />
          </div>
          <div class="form-group">
            <label>描述</label>
            <textarea v-model="uploadSoftware.description" class="form-textarea" rows="3" placeholder="请输入软件描述"></textarea>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="uploadSoftwareFile" :disabled="isUploading">
            {{ isUploading ? '上传中...' : '上传' }}
          </button>
          <button class="btn-secondary" @click="closeSoftwareUploadDialog">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed, watch } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'
import DirectoryTree from './DirectoryTree.vue'

const router = useRouter()
const userStore = useUserStore()
const API_BASE_URL = 'http://192.168.156.20:8080/api'

// 状态
const activeMainTab = ref<'model' | 'software'>('model')
const currentLibId = ref(1)
const directories = ref<any[]>([])
const currentFiles = ref<any[]>([])
const softwareList = ref<any[]>([])
const isUploading = ref(false)
const isMigrating = ref(false)

// 文件数量映射表（目录ID -> 文件数量）
const fileCountMap = ref<Record<number, number>>({})
const rootFileCount = ref(0)

// 当前选中的目录ID（0表示根目录）
const selectedDirId = ref(0)

// 弹窗状态
const showUploadDialog = ref(false)
const showModelInfoDialog = ref(false)
const showMigrateDialog = ref(false)
const showSoftwareUploadDialog = ref(false)

// 当前操作的模型
const currentModelInfo = ref<any>({})
const migratingModel = ref<any>(null)

// 上传表单
const fileInput = ref<HTMLInputElement>()
const selectedFile = ref<File | null>(null)
const uploadModel = ref({
  name: '',
  version: '',
  startCmd: '',
  stopCmd: '',
  description: '',
  libId: 1,
  directoryId: 0
})

// 软件上传表单
const softwareFileInput = ref<HTMLInputElement>()
const selectedSoftwareFile = ref<File | null>(null)
const uploadSoftware = ref({
  name: '',
  version: '',
  installCmd: '',
  uninstallCmd: '',
  startCmd: '',
  stopCmd: '',
  description: ''
})

const allDirectories = ref<any[]>([])

// 上传时可选择的目录列表
const uploadDirectories = ref<any[]>([])

// 计算根目录
const rootDirectories = computed(() => {
  return allDirectories.value.filter(dir => dir.parentId === 0)
})

// 当前选中的目录名称
const currentDirName = computed(() => {
  if (selectedDirId.value === 0) return '根目录'
  const dir = allDirectories.value.find(d => d.id === selectedDirId.value)
  return dir?.name || ''
})

// 计算所有文件的目录数量映射
const calculateFileCounts = async () => {
  try {
    console.log('Calculating file counts for libId:', currentLibId.value)
    const response = await fetch(`${API_BASE_URL}/model/files?libId=${currentLibId.value}`, {
      headers: getHeaders()
    })
    if (response.ok) {
      const allFiles = await response.json()
      console.log('Received files:', allFiles.length, 'files')
      
      const newCounts: Record<number, number> = {}
      let rootCount = 0
      
      allFiles.forEach((file: any) => {
        let rawDirId = file.directoryId ?? file.dirId ?? 0
        let dirId: number
        if (rawDirId === null || rawDirId === undefined || rawDirId === '') {
          dirId = 0
        } else if (typeof rawDirId === 'string') {
          dirId = parseInt(rawDirId, 10) || 0
        } else {
          dirId = Number(rawDirId) || 0
        }
        if (Number.isNaN(dirId)) {
          dirId = 0
        }
        console.log(`File ${file.name}: directoryId = ${dirId} (original directoryId=${file.directoryId}, dirId=${file.dirId})`)
        if (dirId === 0) {
          rootCount++
        } else {
          newCounts[dirId] = (newCounts[dirId] || 0) + 1
        }
      })
      
      // 直接赋值新对象，触发响应式更新
      fileCountMap.value = newCounts
      rootFileCount.value = rootCount
      
      console.log('File counts updated:', { rootCount, directoryCounts: newCounts })
    } else {
      console.error('Failed to fetch files for count calculation:', response.status)
    }
  } catch (error) {
    console.error('Failed to calculate file counts:', error)
  }
}

// 加载上传时可选择的目录列表
const loadUploadDirectories = async () => {
  try {
    const response = await fetch(`${API_BASE_URL}/model/directories?libId=${uploadModel.value.libId}`, {
      headers: getHeaders()
    })
    if (response.ok) {
      uploadDirectories.value = await response.json()
    }
  } catch (error) {
    console.error('Failed to load upload directories:', error)
  }
}

// 加载目录（扁平化存储）
const loadDirectories = async () => {
  try {
    console.log('Loading directories for libId:', currentLibId.value)
    const response = await fetch(`${API_BASE_URL}/model/directories?libId=${currentLibId.value}`, {
      headers: getHeaders()
    })
    if (response.ok) {
      const dirs = await response.json()
      console.log('Loaded directories:', dirs.length, 'directories')
      // 存储为扁平列表
      allDirectories.value = dirs
      console.log('allDirectories updated:', allDirectories.value)
    } else {
      console.error('Failed to load directories:', response.status)
    }
  } catch (error) {
    console.error('Failed to load directories:', error)
  }
}

// 创建目录后重新加载
const createDirectory = async () => {
  const name = prompt('请输入目录名称')
  if (!name) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/directories`, {
      method: 'POST',
      headers: {
        ...getHeaders(),
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        libId: currentLibId.value,
        parentId: 0,
        name
      })
    })
    if (response.ok) {
      await loadDirectories()
      await calculateFileCounts()  // 更新文件数量统计
      alert('目录创建成功')
    } else {
      alert('创建目录失败')
    }
  } catch (error) {
    console.error('Failed to create directory:', error)
    alert('创建目录失败')
  }
}

// 修改删除目录函数
const deleteDirectory = async (dir: any) => {
  if (!confirm(`确定要删除目录 "${dir.name}" 及其所有内容吗？`)) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/directories/${dir.id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    if (response.ok) {
      await loadDirectories()
      await calculateFileCounts()  // 更新文件数量统计
      if (selectedDirId.value === dir.id) {
        selectedDirId.value = 0
        await loadFiles()
      }
      alert('目录删除成功')
    } else {
      alert('删除目录失败')
    }
  } catch (error) {
    console.error('Failed to delete directory:', error)
    alert('删除目录失败')
  }
}

// 修改重命名目录函数
const renameDirectory = async (dir: any) => {
  const newName = prompt('请输入新名称', dir.name)
  if (!newName || newName === dir.name) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/directories/${dir.id}`, {
      method: 'PUT',
      headers: {
        ...getHeaders(),
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ name: newName })
    })
    if (response.ok) {
      await loadDirectories()
      await calculateFileCounts()  // 更新文件数量统计
      alert('重命名成功')
    } else {
      alert('重命名失败')
    }
  } catch (error) {
    console.error('Failed to rename directory:', error)
    alert('重命名失败')
  }
}

// 修改切换模型库函数，重置选中的目录
const switchLib = async (libId: number) => {
  console.log('Switching to lib:', libId)
  currentLibId.value = libId
  selectedDirId.value = 0  // 重置为根目录
  await loadDirectories()
  await calculateFileCounts()  // 更新文件数量统计
  await loadFiles()
}
// 迁移目标
const migrateTarget = ref({
  libId: 0,
  dirId: 0
})
const migrateDirectories = ref<any[]>([])

// 模型库列表
const modelLibs = ref([
  { id: 1, name: '开发库' },
  { id: 2, name: '受控库' },
  { id: 3, name: '产品库' }
])

const roleClass = computed(() => {
  if (!userStore.userInfo) return ''
  switch (userStore.userInfo.role) {
    case 0: return 'super-admin'
    case 1: return 'admin'
    case 2: return 'designer'
    default: return 'viewer'
  }
})

const getHeaders = () => {
  return {
    'Authorization': `Bearer ${userStore.token}`
  }
}

const formatFileSize = (bytes: number) => {
  if (!bytes) return '-'
  const sizes = ['B', 'KB', 'MB', 'GB']
  const i = Math.floor(Math.log(bytes) / Math.log(1024))
  return (bytes / Math.pow(1024, i)).toFixed(2) + ' ' + sizes[i]
}

const formatDate = (timestamp: number) => {
  if (!timestamp) return '-'
  return new Date(timestamp * 1000).toLocaleString('zh-CN')
}

const loadFiles = async () => {
  try {
    const url = selectedDirId.value === 0 
      ? `${API_BASE_URL}/model/files?libId=${currentLibId.value}`
      : `${API_BASE_URL}/model/files?libId=${currentLibId.value}&dirId=${selectedDirId.value}`
    
    const response = await fetch(url, {
      headers: getHeaders()
    })
    if (response.ok) {
      currentFiles.value = await response.json()
    }
  } catch (error) {
    console.error('Failed to load files:', error)
  }
}


const loadSoftware = async () => {
  try {
    const response = await fetch(`${API_BASE_URL}/software`, {
      headers: getHeaders()
    })
    if (response.ok) {
      softwareList.value = await response.json()
    }
  } catch (error) {
    console.error('Failed to load software:', error)
  }
}

// // 创建目录
// const createDirectory = async () => {
//   const name = prompt('请输入目录名称')
//   if (!name) return
  
//   try {
//     const response = await fetch(`${API_BASE_URL}/model/directories`, {
//       method: 'POST',
//       headers: {
//         ...getHeaders(),
//         'Content-Type': 'application/json'
//       },
//       body: JSON.stringify({
//         libId: currentLibId.value,
//         parentId: 0,
//         name
//       })
//     })
//     if (response.ok) {
//       await loadDirectories()
//     } else {
//       alert('创建目录失败')
//     }
//   } catch (error) {
//     console.error('Failed to create directory:', error)
//     alert('创建目录失败')
//   }
// }

// // 删除目录
// const deleteDirectory = async (dir: any) => {
//   if (!confirm(`确定要删除目录 "${dir.name}" 及其所有内容吗？`)) return
  
//   try {
//     const response = await fetch(`${API_BASE_URL}/model/directories/${dir.id}`, {
//       method: 'DELETE',
//       headers: getHeaders()
//     })
//     if (response.ok) {
//       await loadDirectories()
//       await loadFiles()
//     } else {
//       alert('删除目录失败')
//     }
//   } catch (error) {
//     console.error('Failed to delete directory:', error)
//     alert('删除目录失败')
//   }
// }

// // 重命名目录
// const renameDirectory = async (dir: any) => {
//   const newName = prompt('请输入新名称', dir.name)
//   if (!newName || newName === dir.name) return
  
//   try {
//     const response = await fetch(`${API_BASE_URL}/model/directories/${dir.id}`, {
//       method: 'PUT',
//       headers: {
//         ...getHeaders(),
//         'Content-Type': 'application/json'
//       },
//       body: JSON.stringify({ name: newName })
//     })
//     if (response.ok) {
//       await loadDirectories()
//     } else {
//       alert('重命名失败')
//     }
//   } catch (error) {
//     console.error('Failed to rename directory:', error)
//     alert('重命名失败')
//   }
// }

// 选择目录
const selectDirectory = async (dirId: number) => {
  selectedDirId.value = dirId
  await loadFiles()
}
// 上传模型
const openUploadDialog = () => {
  uploadModel.value = {
    name: '',
    version: '',
    startCmd: '',
    stopCmd: '',
    description: '',
    libId: currentLibId.value,
    directoryId: 0
  }
  selectedFile.value = null
  
  // 加载当前库的目录列表
  loadUploadDirectories()
  
  showUploadDialog.value = true
}

const onFileSelected = (e: Event) => {
  const target = e.target as HTMLInputElement
  if (target.files && target.files[0]) {
    selectedFile.value = target.files[0]
  }
}

const onLibChange = async () => {
  uploadModel.value.directoryId = 0 // 重置目录选择
  await loadUploadDirectories()
}

const uploadModelFile = async () => {
  if (!selectedFile.value) {
    alert('请选择文件')
    return
  }
  if (!uploadModel.value.name) {
    alert('请输入模型名称')
    return
  }
  
  console.log('Starting upload with:', {
    fileName: selectedFile.value.name,
    fileSize: selectedFile.value.size,
    libId: uploadModel.value.libId,
    directoryId: uploadModel.value.directoryId,
    name: uploadModel.value.name
  })
  
  isUploading.value = true
  
  const formData = new FormData()
  formData.append('file', selectedFile.value)
  formData.append('name', uploadModel.value.name)
  formData.append('version', uploadModel.value.version || '')
  formData.append('startCmd', uploadModel.value.startCmd || '')
  formData.append('stopCmd', uploadModel.value.stopCmd || '')
  formData.append('description', uploadModel.value.description || '')
  formData.append('libId', String(uploadModel.value.libId))
  formData.append('directoryId', String(uploadModel.value.directoryId))
  
  // 调试：打印 FormData 内容
  console.log('FormData contents:')
  for (const [key, value] of formData.entries()) {
    if (value instanceof File) {
      console.log(`${key}: File(${value.name}, ${value.size} bytes)`)
    } else {
      console.log(`${key}: ${value}`)
    }
  }
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/files`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${userStore.token}`
        // 不要设置 Content-Type，让浏览器自动设置 multipart/form-data boundary
      },
      body: formData
    })
    
    const responseText = await response.text()
    console.log('Response status:', response.status)
    console.log('Response body:', responseText)
    
    if (response.ok) {
      alert('上传成功')
      closeUploadDialog()
    await calculateFileCounts()  // 更新文件数量统计
    await loadFiles()  // 刷新文件列表
    await loadDirectories()  // 刷新目录
    } else {
      let errorMsg = responseText
      try {
        const errorJson = JSON.parse(responseText)
        errorMsg = errorJson.error || errorJson.message || responseText
      } catch (e) {
        // 不是 JSON 格式
      }
      alert(`上传失败: ${errorMsg}`)
    }
  } catch (error) {
    console.error('Upload error:', error)
    alert(`上传失败: ${error}`)
  } finally {
    isUploading.value = false
  }
}

// 下载模型
const downloadModel = async (file: any) => {
  try {
    const response = await fetch(`${API_BASE_URL}/model/files/${file.id}/download`, {
      headers: getHeaders()
    })
    
    if (response.ok) {
      const blob = await response.blob()
      const url = URL.createObjectURL(blob)
      const a = document.createElement('a')
      a.href = url
      a.download = file.fileName
      document.body.appendChild(a)
      a.click()
      document.body.removeChild(a)
      URL.revokeObjectURL(url)
    } else {
      alert('下载失败')
    }
  } catch (error) {
    console.error('Failed to download:', error)
    alert('下载失败')
  }
}

// 删除模型
const deleteModelFile = async (file: any) => {
  if (!confirm(`确定要删除模型 "${file.name}" 吗？`)) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/files/${file.id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    if (response.ok) {
      await calculateFileCounts()  // 更新文件数量统计
      await loadFiles()
      alert('删除成功')
    } else {
      alert('删除失败')
    }
  } catch (error) {
    console.error('Failed to delete:', error)
    alert('删除失败')
  }
}

// 显示模型详情
const showModelInfo = (file: any) => {
  currentModelInfo.value = file
  showModelInfoDialog.value = true
}

// 迁移模型
const openMigrateDialog = async (file: any) => {
  migratingModel.value = file
  migrateTarget.value = { libId: 0, dirId: 0 }
  
  // 加载目标库的目录
  if (migrateTarget.value.libId) {
    try {
      const response = await fetch(`${API_BASE_URL}/model/directories?libId=${migrateTarget.value.libId}`, {
        headers: getHeaders()
      })
      if (response.ok) {
        migrateDirectories.value = await response.json()
      }
    } catch (error) {
      console.error('Failed to load directories:', error)
    }
  }
  
  showMigrateDialog.value = true
}

const confirmMigrate = async () => {
  if (!migrateTarget.value.libId) {
    alert('请选择目标库')
    return
  }
  
  isMigrating.value = true
  
  try {
    const response = await fetch(`${API_BASE_URL}/model/files/${migratingModel.value.id}/migrate`, {
      method: 'POST',
      headers: {
        ...getHeaders(),
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        targetLibId: migrateTarget.value.libId,
        targetDirId: migrateTarget.value.dirId
      })
    })
    
    if (response.ok) {
      alert('迁移成功')
      closeMigrateDialog()
      await calculateFileCounts()  // 更新源库和目标库的文件数量统计
      await loadFiles()
    } else {
      const error = await response.json()
      alert(error.error || '迁移失败')
    }
  } catch (error) {
    console.error('Failed to migrate:', error)
    alert('迁移失败')
  } finally {
    isMigrating.value = false
  }
}

// 软件操作
const openSoftwareUploadDialog = () => {
  uploadSoftware.value = {
    name: '',
    version: '',
    installCmd: '',
    uninstallCmd: '',
    startCmd: '',
    stopCmd: '',
    description: ''
  }
  selectedSoftwareFile.value = null
  showSoftwareUploadDialog.value = true
}

const onSoftwareFileSelected = (e: Event) => {
  const target = e.target as HTMLInputElement
  if (target.files && target.files[0]) {
    selectedSoftwareFile.value = target.files[0]
    if (!uploadSoftware.value.name) {
      uploadSoftware.value.name = target.files[0].name.replace(/\.[^/.]+$/, '')
    }
  }
}

const uploadSoftwareFile = async () => {
  if (!selectedSoftwareFile.value) {
    alert('请选择文件')
    return
  }
  if (!uploadSoftware.value.name) {
    alert('请输入软件名称')
    return
  }
  
  isUploading.value = true
  
  const formData = new FormData()
  formData.append('file', selectedSoftwareFile.value)
  formData.append('name', uploadSoftware.value.name)
  formData.append('version', uploadSoftware.value.version || '')
  formData.append('installCmd', uploadSoftware.value.installCmd || '')
  formData.append('uninstallCmd', uploadSoftware.value.uninstallCmd || '')
  formData.append('startCmd', uploadSoftware.value.startCmd || '')
  formData.append('stopCmd', uploadSoftware.value.stopCmd || '')
  formData.append('description', uploadSoftware.value.description || '')
  
  try {
    const response = await fetch(`${API_BASE_URL}/software`, {
      method: 'POST',
      headers: {
        'Authorization': `Bearer ${userStore.token}`
      },
      body: formData
    })
    
    const responseText = await response.text()
    console.log('Software upload response:', response.status, responseText)
    
    if (response.ok) {
      alert('上传成功')
      closeSoftwareUploadDialog()
      await loadSoftware()
    } else {
      let errorMsg = responseText
      try {
        const errorJson = JSON.parse(responseText)
        errorMsg = errorJson.error || responseText
      } catch (e) {}
      alert(`上传失败: ${errorMsg}`)
    }
  } catch (error) {
    console.error('Failed to upload software:', error)
    alert('上传失败')
  } finally {
    isUploading.value = false
  }
}

const downloadSoftware = async (sw: any) => {
  try {
    const response = await fetch(`${API_BASE_URL}/software/${sw.id}/download`, {
      headers: getHeaders()
    })
    
    if (response.ok) {
      const blob = await response.blob()
      const url = URL.createObjectURL(blob)
      const a = document.createElement('a')
      a.href = url
      a.download = sw.fileName
      document.body.appendChild(a)
      a.click()
      document.body.removeChild(a)
      URL.revokeObjectURL(url)
    } else {
      alert('下载失败')
    }
  } catch (error) {
    console.error('Failed to download:', error)
    alert('下载失败')
  }
}

const showSoftwareInfo = (sw: any) => {
  currentModelInfo.value = sw
  showModelInfoDialog.value = true
}

const deleteSoftware = async (sw: any) => {
  if (!confirm(`确定要删除软件 "${sw.name}" 吗？`)) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/software/${sw.id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    if (response.ok) {
      await loadSoftware()
      alert('删除成功')
    } else {
      alert('删除失败')
    }
  } catch (error) {
    console.error('Failed to delete:', error)
    alert('删除失败')
  }
}

// 关闭弹窗
const closeUploadDialog = () => {
  showUploadDialog.value = false
  if (fileInput.value) fileInput.value.value = ''
}

const closeModelInfoDialog = () => {
  showModelInfoDialog.value = false
  currentModelInfo.value = {}
}

const closeMigrateDialog = () => {
  showMigrateDialog.value = false
  migratingModel.value = null
}

const closeSoftwareUploadDialog = () => {
  showSoftwareUploadDialog.value = false
  if (softwareFileInput.value) softwareFileInput.value.value = ''
}

// 导航
const goBack = () => {
  router.push('/')
}

const handleLogout = () => {
  userStore.logout()
  router.push('/login')
}

// 初始化
onMounted(() => {
userStore.loadUserFromStorage()
  loadDirectories()
  calculateFileCounts()  // 计算文件数量统计
  loadFiles()
  loadSoftware()
})

// 监听 allDirectories 变化
watch(allDirectories, (newVal) => {
  console.log('allDirectories changed:', newVal.length, 'directories')
}, { deep: true })

// 监听 rootDirectories 变化
watch(rootDirectories, (newVal) => {
  console.log('rootDirectories changed:', newVal.length, 'directories')
}, { deep: true })
</script>

<style scoped>
.model-manage-container {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
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

.navbar {
  margin: 20px 24px;
  padding: 12px 24px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  position: relative;
  z-index: 10;
}

.nav-left {
  display: flex;
  align-items: center;
  gap: 16px;
}

.back-icon {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 36px;
  height: 36px;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.2s;
  color: #94a3b8;
}

.back-icon:hover {
  background: rgba(255, 255, 255, 0.1);
  color: white;
}

.logo-icon {
  display: flex;
  align-items: center;
  gap: 8px;
  color: #3b82f6;
}

.logo-text {
  font-size: 20px;
  font-weight: 700;
  background: linear-gradient(135deg, #fff, #94a3b8);
  -webkit-background-clip: text;
  background-clip: text;
  color: transparent;
}

.nav-title {
  font-size: 16px;
  color: #94a3b8;
  padding-left: 16px;
  border-left: 1px solid rgba(255, 255, 255, 0.1);
}

.nav-right {
  display: flex;
  align-items: center;
  gap: 16px;
}

.user-info {
  display: flex;
  align-items: center;
  gap: 8px;
}

.user-name {
  color: white;
  font-size: 14px;
}

.user-role {
  font-size: 11px;
  padding: 2px 8px;
  border-radius: 20px;
}

.user-role.super-admin {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.user-role.admin {
  background: rgba(245, 158, 11, 0.2);
  color: #f59e0b;
}

.user-role.designer {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.user-role.viewer {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.btn-logout {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  color: #cbd5e1;
  font-size: 13px;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-logout:hover {
  background: rgba(239, 68, 68, 0.2);
  border-color: rgba(239, 68, 68, 0.4);
  color: #ef4444;
}

.main-tabs {
  margin: 0 24px 20px;
  padding: 8px;
  display: flex;
  gap: 8px;
  width: auto;
}

.tab {
  flex: 0 0 auto;
  padding: 10px 24px;
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

.lib-tabs {
  display: flex;
  gap: 12px;
  margin-bottom: 20px;
  padding: 0 24px;
}

.lib-tab {
  padding: 8px 20px;
  background: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  color: #94a3b8;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
}

.lib-tab.active {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
  border-color: transparent;
}

.model-manage-area, .software-manage-area {
  flex: 1;
  padding: 0 24px 24px;
  overflow: hidden;
}

.content-layout {
  display: flex;
  gap: 24px;
  height: calc(100% - 60px);
}

.directory-tree {
  width: 280px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.tree-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  font-weight: 600;
  color: white;
}

.tree-content {
  flex: 1;
  overflow-y: auto;
  padding: 12px;
}

.empty-tree {
  text-align: center;
  padding: 40px;
  color: #64748b;
  font-size: 13px;
}

.file-list {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.file-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  font-weight: 600;
  color: white;
}

.btn-upload {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 14px;
  background: linear-gradient(135deg, #10b981, #06b6d4);
  border: none;
  border-radius: 20px;
  color: white;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-upload:hover {
  transform: scale(1.02);
}

.btn-icon-small {
  width: 28px;
  height: 28px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: rgba(255, 255, 255, 0.08);
  border: none;
  color: #94a3b8;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-icon-small:hover {
  background: rgba(59, 130, 246, 0.3);
  color: #3b82f6;
}

.file-table-wrapper, .software-table-wrapper {
  flex: 1;
  overflow: auto;
}

.file-table, .software-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}

.file-table th, .software-table th,
.file-table td, .software-table td {
  padding: 12px 16px;
  text-align: left;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
}

.file-table th, .software-table th {
  color: #94a3b8;
  font-weight: 500;
  background: rgba(0, 0, 0, 0.2);
}

.file-table td, .software-table td {
  color: #e2e8f0;
}

.file-actions {
  display: flex;
  gap: 8px;
}

.btn-download, .btn-migrate, .btn-info, .btn-delete {
  width: 28px;
  height: 28px;
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  border: none;
  cursor: pointer;
  font-size: 14px;
  transition: all 0.2s;
}

.btn-download {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.btn-download:hover {
  background: rgba(16, 185, 129, 0.4);
}

.btn-migrate {
  background: rgba(245, 158, 11, 0.2);
  color: #f59e0b;
}

.btn-migrate:hover {
  background: rgba(245, 158, 11, 0.4);
}

.btn-info {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.btn-info:hover {
  background: rgba(59, 130, 246, 0.4);
}

.btn-delete {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.btn-delete:hover {
  background: rgba(239, 68, 68, 0.4);
}

.empty-table {
  text-align: center;
  padding: 60px !important;
  color: #64748b;
}

.software-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding: 16px;
  background: rgba(0, 0, 0, 0.2);
  border-radius: 16px;
  color: white;
  font-weight: 600;
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
  background: rgba(15, 25, 35, 0.95);
  backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 28px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
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
  padding: 10px 14px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  color: white;
  font-size: 14px;
}

.form-input:focus, .form-textarea:focus, .form-select:focus {
  outline: none;
  border-color: #3b82f6;
}

.form-textarea {
  resize: vertical;
}

.selected-file {
  padding: 8px 12px;
  background: rgba(16, 185, 129, 0.1);
  border-radius: 8px;
  color: #10b981;
  font-size: 13px;
}

.info-item {
  margin-bottom: 16px;
}

.info-item label {
  display: block;
  font-size: 12px;
  font-weight: 500;
  color: #94a3b8;
  margin-bottom: 4px;
}

.info-item span, .info-item p, .info-item code {
  font-size: 14px;
  color: #e2e8f0;
  word-break: break-all;
}

.info-item code {
  display: inline-block;
  background: rgba(0, 0, 0, 0.3);
  padding: 4px 8px;
  border-radius: 6px;
  font-family: monospace;
}

.btn-primary {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  padding: 10px 20px;
  border-radius: 40px;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
}

.btn-secondary {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  padding: 10px 20px;
  border-radius: 40px;
  color: #cbd5e1;
  font-size: 14px;
  cursor: pointer;
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
</style>