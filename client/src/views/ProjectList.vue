<!-- ProjectList.vue (修改后) -->
<template>
  <div class="project-list-container">
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 顶部导航栏 -->
    <div class="navbar glass-panel">
      <div class="nav-left">
        <div class="logo-icon" @click="goBack">
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
        <span class="nav-title">工程管理</span>
      </div>
      <div class="nav-right">
        <div class="user-info" v-if="userStore.userInfo">
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

    <!-- 工程列表主要内容 -->
    <div class="projects-container">
      <!-- 标签页 + 搜索栏 -->
      <div class="projects-header">
        <div class="tabs-wrapper">
          <button 
            :class="['tab-btn', { active: activeTab === 'public' }]"
            @click="activeTab = 'public'"
          >
            公开工程
          </button>
          <button 
            :class="['tab-btn', { active: activeTab === 'private' }]"
            @click="activeTab = 'private'"
          >
            私有工程
          </button>
        </div>
        <div class="search-wrapper">
          <input
            type="text"
            class="search-input"
            v-model="searchKeyword"
            placeholder="🔍 搜索工程名称..."
          />
          <button class="btn-create" @click="openCreateDialog">+ 新建工程</button>
        </div>
      </div>

      <!-- 工程卡片网格 -->
      <div class="projects-grid">
        <div 
          v-for="project in filteredProjects" 
          :key="project.id" 
          class="project-card glass-panel" 
          @click="openProject(project)"
        >
          <div class="project-icon">
            <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
              <path d="M4 4H20V20H4V4Z"/>
              <path d="M8 7H16M8 12H16M8 17H12"/>
            </svg>
          </div>
          <div class="project-info">
            <h3>{{ project.name }}</h3>
            <p class="project-desc">{{ project.description || '无描述' }}</p>
            <div class="project-meta">
              <span class="project-owner">创建者: {{ project.ownerName }}</span>
              <span class="project-time">更新: {{ formatDate(project.updatedAt) }}</span>
              <span v-if="project.isPublic" class="public-badge">公开</span>
              <span v-else class="private-badge">私有</span>
            </div>
          </div>
          <div class="project-actions" @click.stop>
            <button class="btn-icon edit" @click="editProject(project)" title="编辑">✎</button>
            <button class="btn-icon delete" @click="deleteProject(project)" title="删除">×</button>
          </div>
        </div>

        <div v-if="filteredProjects.length === 0" class="empty-state">
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

    <!-- 创建/编辑工程弹窗（保持不变） -->
    <div v-if="showProjectDialog" class="modal-overlay" @click.self="closeProjectDialog">
      <div class="modal-content glass-panel">
        <div class="modal-header">
          <h3>{{ isEditing ? '编辑工程' : '新建工程' }}</h3>
          <button class="modal-close" @click="closeProjectDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>工程名称 *</label>
            <input v-model="editingProject.name" class="form-input" placeholder="请输入工程名称" />
          </div>
          <div class="form-group">
            <label>描述</label>
            <textarea v-model="editingProject.description" class="form-textarea" rows="3" placeholder="请输入工程描述"></textarea>
          </div>
          <div class="form-group">
            <label>
              <input type="checkbox" v-model="editingProject.isPublic" />
              公开工程（其他用户可查看）
            </label>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="saveProject" :disabled="isLoading">
            {{ isLoading ? '保存中...' : '保存' }}
          </button>
          <button class="btn-secondary" @click="closeProjectDialog">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'
import { formatDate } from '@/utils/format'
import { saveProjectInfo, delProjectInfo,getProjectInfo } from '@/utils/api'

const router = useRouter()
const userStore = useUserStore()

const projects = ref<any[]>([])
const isLoading = ref(false)
const showProjectDialog = ref(false)
const isEditing = ref(false)
const activeTab = ref<'public' | 'private'>('public')
const searchKeyword = ref('')

const editingProject = ref({
  id: null,
  name: '',
  description: '',
  isPublic: false
})

// 过滤工程：按可见性 + 搜索关键词
const filteredProjects = computed(() => {
  let list = projects.value
  if (activeTab.value === 'public') {
    list = list.filter(p => p.isPublic === true)
  } else {
    // 私有工程仅显示当前用户创建的
    list = list.filter(p => p.isPublic === false && p.ownerId === userStore.userInfo?.id)
  }
  if (searchKeyword.value) {
    const kw = searchKeyword.value.toLowerCase()
    list = list.filter(p => p.name.toLowerCase().includes(kw))
  }
  return list
})


const fetchProjects = async () => {
  isLoading.value = true
  try {
    const response = await getProjectInfo()
    if (response) {
      projects.value = response
    }
  } catch (error) {
    console.error('Failed to fetch projects:', error)
  } finally {
    isLoading.value = false
  }
}

const openProject = (project: any) => {
  sessionStorage.setItem('currentProject', JSON.stringify(project))
  router.push('/model-designer')
}

const openCreateDialog = () => {
  isEditing.value = false
  editingProject.value = { id: null, name: '', description: '', isPublic: false }
  showProjectDialog.value = true
}

const editProject = (project: any) => {
  isEditing.value = true
  editingProject.value = {
    id: project.id,
    name: project.name,
    description: project.description || '',
    isPublic: project.isPublic
  }
  showProjectDialog.value = true
}

const saveProject = async () => {
  if (!editingProject.value.name.trim()) {
    alert('请输入工程名称')
    return
  }
  isLoading.value = true
  try {
    const url = isEditing.value ? `/projects/${editingProject.value.id}` : `/projects`
    const method = isEditing.value ? 'PUT' : 'POST'
    const response = await saveProjectInfo(url, method, editingProject.value)
    if (response) {
      alert(isEditing.value ? '工程更新成功' : '工程创建成功')
      closeProjectDialog()
      fetchProjects()
    } else {
      alert('操作失败')
    }
  } catch (error) {
    console.error('Failed to save project:', error)
    alert('操作失败')
  } finally {
    isLoading.value = false
  }
}

const deleteProject = async (project: any) => {
  if (!confirm(`确定要删除工程 "${project.name}" 吗？此操作不可恢复。`)) return
  isLoading.value = true
  try {
    const response = await delProjectInfo(`/projects/${project.id}`, 'DELETE')

    if (response) {
      alert('工程删除成功')
      fetchProjects()
    } else {
      alert('删除失败')
    }
  } catch (error) {
    console.error('Failed to delete project:', error)
    alert('删除失败')
  } finally {
    isLoading.value = false
  }
}

const goBack = () => router.push('/')

const handleLogout = () => {
  userStore.logout()
  router.push('/login')
}
const closeProjectDialog = () => { showProjectDialog.value = false }

onMounted(() => {
  userStore.loadUserFromStorage()
  fetchProjects()
})
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.project-list-container, .simulation-run-container {
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

/* 导航栏 */
.navbar {
  margin: 20px 24px;
  padding: 12px 24px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  z-index: 10;
  position: relative;
}

.nav-left { display: flex; align-items: center; gap: 16px; }
.logo-icon { display: flex; align-items: center; gap: 8px; color: #3b82f6; cursor: pointer; }
.logo-text { font-size: 20px; font-weight: 700; background: linear-gradient(135deg, #fff, #94a3b8); -webkit-background-clip: text; background-clip: text; color: transparent; }
.nav-title { font-size: 16px; color: #94a3b8; padding-left: 16px; border-left: 1px solid rgba(255,255,255,0.1); }

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

.projects-container, .project-selection {
  position: relative;
  z-index: 10;
  max-width: 1400px;
  margin: 0 auto;
  padding: 0 24px;
  height: calc(100% - 80px);
  overflow-y: auto;
}

/* 标签页 + 搜索 + 按钮 */
.projects-header, .selection-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
  flex-wrap: wrap;
  gap: 16px;
}


.tabs-wrapper, .left-area {
  display: flex;
  gap: 8px;
  align-items: center;
}

.tab-btn, .back-home-btn {
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
  background: rgba(59,130,246,0.15);
  border: 1px solid rgba(59,130,246,0.3);
}
.back-home-btn {
  background: rgba(255,255,255,0.08);
  margin-right: 12px;
}
.back-home-btn:hover {
  background: rgba(255,255,255,0.15);
}

.search-wrapper, .header-actions {
  display: flex;
  gap: 12px;
  align-items: center;
}

.search-input {
  background: rgba(0,0,0,0.4);
  border: 1px solid rgba(255,255,255,0.1);
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

.btn-primary, .btn-secondary, .btn-create {
  padding: 8px 20px;
  border-radius: 40px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
}

.btn-primary, .btn-create {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
}
.btn-secondary {
  background: rgba(255,255,255,0.08);
  border: 1px solid rgba(255,255,255,0.15);
  color: #cbd5e1;
}


.projects-header h2 {
  color: white;
  font-size: 24px;
  font-weight: 600;
}

.btn-create {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  padding: 10px 20px;
  border-radius: 40px;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-create:hover {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
}

.projects-grid, .project-list {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(360px, 1fr));
  gap: 20px;
}

.project-card {
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
  padding: 20px;
  transition: all 0.3s ease;
  cursor: pointer;
  display: flex;
  position: relative;
}
.project-card:hover {
  transform: translateY(-4px);
  border-color: rgba(59,130,246,0.4);
  box-shadow: 0 12px 32px rgba(0,0,0,0.3);
}

.project-icon {
  width: 60px;
  height: 60px;
  background: rgba(59,130,246,0.15);
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #3b82f6;
  margin-right: 16px;
  flex-shrink: 0;
}

.project-info {
  flex: 1;
}

.project-info h3 {
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
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.project-meta {
  display: flex;
  gap: 12px;
  font-size: 11px;
  color: #64748b;
}

.project-owner { color: #3b82f6; }
.public-badge { background: rgba(16,185,129,0.15); color: #10b981; padding: 2px 6px; border-radius: 20px; }
.private-badge { background: rgba(139,92,246,0.15); color: #a78bfa; padding: 2px 6px; border-radius: 20px; }

.project-owner {
  color: #3b82f6;
}

.project-public {
  background: rgba(16, 185, 129, 0.15);
  color: #10b981;
  padding: 2px 6px;
  border-radius: 10px;
}

.project-actions {
  position: absolute;
  top: 12px;
  right: 12px;
  display: flex;
  gap: 4px;
  opacity: 0;
  transition: opacity 0.2s;
}

.project-card:hover .project-actions {
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

.empty-state {
  text-align: center;
  padding: 80px;
  color: #64748b;
}

.btn-icon.delete:hover {
  background: rgba(239, 68, 68, 0.4);
}

.empty-projects {
  text-align: center;
  padding: 80px;
  color: #64748b;
}

.empty-illustration svg {
  color: #334155;
  margin-bottom: 16px;
}

.empty-projects p {
  font-size: 16px;
  margin-bottom: 8px;
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
  width: 450px;
  max-width: 90%;
  max-height: 80vh;
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

.form-input, .form-textarea {
  width: 100%;
  padding: 10px 14px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  color: white;
  font-size: 14px;
}

.form-input:focus, .form-textarea:focus {
  outline: none;
  border-color: #3b82f6;
}

.form-textarea {
  resize: vertical;
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