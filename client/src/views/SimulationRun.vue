<template>
  <div class="simulation-run-container">
    <!-- 背景装饰 -->
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 工程选择界面 -->
    <div class="project-selection-full">
      <!-- 顶部导航栏 -->
      <div class="navbar glass-panel">
        <div class="nav-left">
          <div class="logo-icon" @click="goHome">
            <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M19 12H5M12 19l-7-7 7-7"/>
            </svg>
          </div>
          <div class="logo-icon">
            <svg width="32" height="32" viewBox="0 0 24 24" fill="none">
              <path d="M4 4H20V20H4V4Z" stroke="currentColor" stroke-width="1.5"/>
              <path d="M8 2V6M16 2V6M4 10H20" stroke="currentColor" stroke-width="1.5"/>
              <path d="M12 14L12 18M9 15L15 15" stroke="currentColor" stroke-width="1.5"/>
            </svg>
            <span class="logo-text">ZRSIM</span>
          </div>
          <span class="nav-title">仿真运行 - 选择工程</span>
        </div>
        <div class="nav-right">
          <div class="user-info" v-if="userStore && userStore.userInfo">
            <span class="user-name">{{ userStore.userInfo.username }}</span>
            <span class="user-role" :class="userStore.roleClass">{{ userStore.userInfo.roleName }}</span>
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

      <!-- 主要内容区 -->
      <div class="projects-container">
        <!-- 标签页 + 搜索 + 新建按钮 -->
        <div class="projects-header">
          <div class="tabs-wrapper">
            <button 
              :class="['tab-btn', { active: curProject.projectTab === 'public' }]"
              @click="curProject.projectTab = 'public'"
            >
              公开工程
            </button>
            <button 
              :class="['tab-btn', { active: curProject.projectTab === 'private' }]"
              @click="curProject.projectTab = 'private'"
            >
              私有工程
            </button>
          </div>
          <div class="search-wrapper">
            <input
              type="text"
              class="search-input"
              v-model="curProject.projectSearch"
              placeholder="🔍 搜索工程名称..."
            />
            <button @click="showCreateProject = true" class="btn-primary">+ 新建工程</button>
            <button @click="refreshProjects" class="btn-secondary">刷新</button>
          </div>
        </div>

        <!-- 工程卡片网格 -->
       <div class="projects-grid">
        <div 
          v-for="project in curProject.filteredProjects"
          :key="project.id"
          class="project-card glass-panel"
        >
          <!-- 上方：工程信息（图标 + 详情） -->
          <div class="project-info-area">
            <div class="project-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <path d="M4 4H20V20H4V4Z"/>
                <path d="M8 7H16M8 12H16M8 17H12"/>
              </svg>
            </div>
            <div class="project-details">
              <h3>{{ project.name }}</h3>
              <p class="project-desc">{{ project.description || '无描述' }}</p>
              <div class="project-meta">
                <span class="project-owner">创建者: {{ project.creator }}</span>
                <span class="project-time">创建: {{ project.createTime }}</span>
                <span :class="['visibility-badge', project.visibility]">
                  {{ project.visibility === 'public' ? '公开' : '私有' }}
                </span>
              </div>
            </div>
          </div>

          <!-- 中部：运行实例列表（宽度拉长，横向或网格布局） -->
          <div v-if="project.runningInstances && project.runningInstances.length > 0" class="instance-list-wide">
            <div class="instances-title">运行实例</div>
            <div class="instance-items">
              <div 
                v-for="instance in project.runningInstances" 
                :key="instance.id"
                class="instance-item-wide"
              >
                <span class="instance-id-wide">{{ instance.id }}</span>
                <span class="instance-runner-wide">{{ instance.startedBy }}</span>
                <button @click="loadInstance(project, instance)" class="load-instance-wide">
                  加载
                </button>
              </div>
            </div>
          </div>

          <!-- 底部：进入工程按钮（全宽或宽按钮） -->
          <div class="project-bottom-action">
            <button 
              @click="enterProject(project)" 
              class="enter-btn-full"
              :disabled="project.lockedBy && project.lockedBy !== currentUser"
              :title="getLockTip(project)"
            >
              {{ project.lockedBy && project.lockedBy !== currentUser ? '只读进入' : '进入工程' }}
            </button>
          </div>
        </div>
      </div>

        <!-- 空状态 -->
        <div v-if="curProject.filteredProjects.length === 0" class="empty-state">
          <div class="empty-illustration">
            <svg width="80" height="80" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.2">
              <path d="M4 4H20V20H4V4Z"/>
              <path d="M8 7H16M8 12H16M8 17H12"/>
            </svg>
          </div>
          <p>暂无工程</p>
          <span>点击"新建工程"创建第一个工程</span>
        </div>
      </div>
    </div>

    <!-- 新建工程弹窗 -->
    <div v-if="showCreateProject" class="modal-overlay" @click.self="showCreateProject = false">
      <div class="modal-content">
        <h3>新建工程</h3>
        <input v-model="newProjectName" placeholder="工程名称" class="modal-input" />
        <div class="modal-radio">
          <label><input type="radio" value="public" v-model="newProjectVisibility" /> 公开工程</label>
          <label><input type="radio" value="private" v-model="newProjectVisibility" /> 私有工程</label>
        </div>
        <div class="modal-actions">
          <button @click="createProject" class="btn-primary">创建</button>
          <button @click="showCreateProject = false" class="btn-secondary">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '@/stores/user'
import { useCurProjectStore } from "@/stores/curProjectInfo"
import { createSimProject,getSimProjectList } from '@/utils/task'

const router = useRouter()
const userStore = useUserStore()
const curProject = useCurProjectStore()

const showCreateProject = ref(false)
const newProjectName = ref('')
const newProjectVisibility = ref<'public' | 'private'>('public')
const currentUser = ref(userStore.userInfo?.username || 'admin')

// Initialize mock data on store's projects if empty
onMounted(() => {
  if (curProject.projects.length === 0) {
    curProject.projects = [
      {
        id: 1,
        name: '智能电网仿真工程',
        visibility: 'public',
        creator: 'zhangsan',
        createTime: '2024-01-15',
        lockedBy: null,
        lockedAt: null,
        runningInstances: [
          { id: 'inst_001', startedBy: 'lisi', startTime: '2024-01-20 10:30:00', status: 'running' }
        ],
        designData: null
      },
      {
        id: 2,
        name: '自动驾驶测试工程',
        visibility: 'public',
        creator: 'wangwu',
        createTime: '2024-02-10',
        lockedBy: null,
        lockedAt: null,
        runningInstances: [],
        designData: null
      },
      {
        id: 3,
        name: '工业机器人仿真',
        visibility: 'private',
        creator: 'lisi',
        createTime: '2024-03-05',
        lockedBy: null,
        lockedAt: null,
        runningInstances: [
          { id: 'inst_003', startedBy: 'lisi', startTime: '2024-03-10 14:20:00', status: 'running' }
        ],
        designData: null
      }
    ]
  }
})


function goHome() {
  router.push('/')
}

function handleLogout() {
  userStore.logout()
  router.push('/login')
}


function refreshProjects() {
  const mockData = [
    {
      id: 1, name: '智能电网仿真工程', visibility: 'public', creator: 'zhangsan',
      createTime: '2024-01-15', lockedBy: null, lockedAt: null,
      runningInstances: [{ id: 'inst_001', startedBy: 'lisi', startTime: '2024-01-20 10:30:00', status: 'running' }],
      designData: null
    },
    {
      id: 2, name: '自动驾驶测试工程', visibility: 'public', creator: 'wangwu',
      createTime: '2024-02-10', lockedBy: null, lockedAt: null, runningInstances: [],
      designData: null
    },
    {
      id: 3, name: '工业机器人仿真', visibility: 'private', creator: 'lisi',
      createTime: '2024-03-05', lockedBy: null, lockedAt: null,
      runningInstances: [{ id: 'inst_003', startedBy: 'lisi', startTime: '2024-03-10 14:20:00', status: 'running' }],
      designData: null
    }
  ]
  curProject.projects = mockData
}

const createProject = async () => {
  if (!newProjectName.value.trim()) {
    alert('请输入工程名称')
    return
  }

  const response =await createSimProject({
    name:newProjectName.value,
    description:"",
    content:"",
    visibility:newProjectVisibility.value
  })

  console.log(response)
  
  showCreateProject.value = false
  newProjectName.value = ''
}

const getProjets = async ()=>{
  const  response = await getSimProjectList();
  console.log(response) 
}

function getLockTip(project: any) {
  if (project.lockedBy && project.lockedBy !== currentUser.value) {
    return `${project.lockedBy} 正在编辑此工程，将以只读模式打开`
  }
  return ''
}

function tryLockProject(project: any, readOnly = false) {
  if (readOnly) return false
  if (project.lockedBy && project.lockedBy !== currentUser.value) {
    return false
  }
  if (!project.lockedBy) {
    project.lockedBy = currentUser.value
    project.lockedAt = new Date().toISOString()
    return true
  }
  return false
}


function enterProject(project: any) {
  const canEdit = tryLockProject(project, false)
  curProject.selectedProject = { ...project }
  curProject.currentInstance = null
  curProject.isReadOnly = !canEdit

  router.push({
    name: 'SimuDesign',
    params: { id: project.id }
  })
}

function loadInstance(project: any, instance: any) {
  curProject.isReadOnly = true
  curProject.selectedProject = { ...project }
  curProject.currentInstance = { ...instance }

  router.push({
    name: 'SimuDesign',
    params: { id: project.id }
  })
}

onMounted(()=>{
  getProjets()
})

</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.simulation-run-container {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
  font-family: 'Segoe UI', 'Inter', system-ui, sans-serif;
  overflow: hidden;
}

.bg-decoration {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
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

/* ========== 顶部导航栏 ========== */
.navbar {
  margin: 20px 24px;
  padding: 12px 24px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  z-index: 10;
  position: relative;
}
.nav-left {
  display: flex;
  align-items: center;
  gap: 16px;
}
.logo-icon {
  display: flex;
  align-items: center;
  gap: 8px;
  color: #3b82f6;
  cursor: pointer;
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
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  padding: 8px 16px;
  color: #cbd5e1;
  cursor: pointer;
  transition: all 0.2s;
}
.btn-logout:hover {
  background: rgba(239, 68, 68, 0.2);
  border-color: rgba(239, 68, 68, 0.4);
  color: #ef4444;
}

/* ========== 工程选择界面 ========== */
.project-selection-full {
  position: relative;
  z-index: 10;
  height: 100vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}
.projects-container {
  flex: 1;
  overflow-y: auto;
  padding: 24px;
}
.projects-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
  flex-wrap: wrap;
  gap: 16px;
}
.tabs-wrapper {
  display: flex;
  gap: 8px;
}
.tab-btn {
  padding: 8px 20px;
  background: transparent;
  border: none;
  color: #94a3b8;
  font-size: 14px;
  cursor: pointer;
  border-radius: 40px;
  transition: all 0.2s;
}
.tab-btn.active {
  color: #3b82f6;
  background: rgba(59, 130, 246, 0.15);
  border: 1px solid rgba(59, 130, 246, 0.3);
}
.search-wrapper {
  display: flex;
  gap: 12px;
  align-items: center;
}
.search-input {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  padding: 8px 16px;
  width: 240px;
  color: white;
  font-size: 13px;
}
.search-input:focus {
  outline: none;
  border-color: #3b82f6;
}
.btn-primary, .btn-secondary {
  padding: 8px 20px;
  border-radius: 40px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
}
.btn-primary {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
}
.btn-secondary {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  color: #cbd5e1;
}
.btn-primary:hover, .btn-secondary:hover {
  transform: translateY(-1px);
}

/* 卡片网格 — 一行三个 */
.projects-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 20px;
}
.project-card {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  height: auto;
  min-height: 260px;
  padding: 20px;
  transition: all 0.3s ease;
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
}
.project-card:hover {
  transform: translateY(-4px);
  border-color: rgba(59, 130, 246, 0.4);
  box-shadow: 0 12px 32px rgba(0, 0, 0, 0.3);
}

/* 卡片内布局 */
.project-info-area {
  display: flex;
  gap: 16px;
  margin-bottom: 16px;
}
.project-icon {
  width: 48px;
  height: 48px;
  background: rgba(59, 130, 246, 0.15);
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #3b82f6;
  flex-shrink: 0;
}
.project-details {
  flex: 1;
  overflow: hidden;
}
.project-details h3 {
  color: white;
  font-size: 16px;
  font-weight: 600;
  margin-bottom: 4px;
}
.project-desc {
  color: #94a3b8;
  font-size: 12px;
  margin-bottom: 8px;
  display: -webkit-box;
  line-clamp: 2;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
}
.project-meta {
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  font-size: 11px;
  color: #64748b;
}
.project-owner {
  color: #3b82f6;
}
.visibility-badge {
  padding: 2px 6px;
  border-radius: 12px;
  font-size: 10px;
}
.visibility-badge.public {
  background: rgba(16, 185, 129, 0.15);
  color: #10b981;
}
.visibility-badge.private {
  background: rgba(139, 92, 246, 0.15);
  color: #a78bfa;
}

/* 运行实例列表（横向宽区域） */
.instance-list-wide {
  background: rgba(0, 0, 0, 0.3);
  border-radius: 16px;
  padding: 12px;
  margin: 8px 0;
  width: 100%;
}
.instances-title {
  font-size: 12px;
  font-weight: 600;
  color: #cbd5e1;
  margin-bottom: 10px;
}
.instance-items {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.instance-item-wide {
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
  padding: 8px 12px;
  width: 100%;
}
.instance-id-wide {
  font-family: monospace;
  color: #fbbf24;
  font-size: 12px;
  font-weight: 500;
}
.instance-runner-wide {
  color: #94a3b8;
  font-size: 11px;
  flex: 1;
  margin-left: 12px;
}
.load-instance-wide {
  background: rgba(245, 158, 11, 0.2);
  border: 1px solid rgba(245, 158, 11, 0.4);
  border-radius: 20px;
  color: #fbbf24;
  font-size: 11px;
  padding: 4px 12px;
  cursor: pointer;
  transition: all 0.2s;
}
.load-instance-wide:hover {
  background: rgba(245, 158, 11, 0.4);
}

/* 底部进入工程按钮（全宽） */
.project-bottom-action {
  margin-top: auto;
  width: 100%;
}
.enter-btn-full {
  width: 100%;
  padding: 10px 0;
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  border-radius: 40px;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}
.enter-btn-full:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
}
.enter-btn-full:disabled {
  background: rgba(100, 116, 139, 0.5);
  cursor: not-allowed;
}

.empty-state {
  text-align: center;
  padding: 80px;
  color: #64748b;
}
.empty-illustration svg {
  color: #334155;
  margin-bottom: 16px;
}

/* 响应式 */
@media (max-width: 1200px) {
  .projects-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}
@media (max-width: 768px) {
  .projects-grid {
    grid-template-columns: 1fr;
  }
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
  border-radius: 32px;
  width: 450px;
  max-width: 90%;
  padding: 24px;
}
.modal-content h3 {
  color: white;
  font-size: 20px;
  margin-bottom: 16px;
}
.modal-input {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  padding: 12px 16px;
  width: 100%;
  color: white;
  margin: 16px 0;
}
.modal-radio {
  display: flex;
  gap: 24px;
  margin: 16px 0;
  color: #cbd5e1;
}
.modal-actions {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  margin-top: 20px;
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