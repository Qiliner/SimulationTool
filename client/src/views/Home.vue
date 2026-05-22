<template>
  <div class="home-container">
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 顶部导航栏 -->
    <div class="navbar glass-panel">
      <div class="nav-left">
        <div class="logo-icon">
          <svg width="32" height="32" viewBox="0 0 24 24" fill="none">
            <path d="M4 4H20V20H4V4Z" stroke="currentColor" stroke-width="1.5"/>
            <path d="M8 2V6M16 2V6M4 10H20" stroke="currentColor" stroke-width="1.5"/>
            <path d="M12 14L12 18M9 15L15 15" stroke="currentColor" stroke-width="1.5"/>
          </svg>
          <span class="logo-text">ZRSIM</span>
        </div>
        <span class="nav-title">模型设计平台</span>
      </div>
      <div class="nav-right">
        <div class="user-info" v-if="userStore.userInfo">
          <span class="user-name">{{ userStore.userInfo.username }}</span>
          <span class="user-role" :class="userStore.roleClass">{{ userStore.userInfo.roleName }}</span>
        </div>
        <div class="user-info" v-else>
          <span class="user-name">未登录</span>
        </div>
        <button class="btn-user" @click="showUserManagement = true">
          <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/>
            <circle cx="12" cy="7" r="4"/>
          </svg>
          用户管理
        </button>
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

    <!-- 四个大模块 -->
    <div class="modules-container">
      <div class="module-card glass-panel" @click="goToModelDesign">
        <div class="module-icon model-design">
          <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <rect x="3" y="3" width="18" height="18" rx="2"/>
            <path d="M8 7H16M8 12H16M8 17H12"/>
          </svg>
        </div>
        <h3>模型设计</h3>
        <p>可视化设计数据结构和模型</p>
        <span class="module-status finished">已实现</span>
      </div>

      <div class="module-card glass-panel" @click="goToModelManage">
        <div class="module-icon model-manage">
          <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M12 2L2 7L12 12L22 7L12 2Z"/>
            <path d="M2 17L12 22L22 17"/>
            <path d="M2 12L12 17L22 12"/>
          </svg>
        </div>
        <h3>模型管理</h3>
        <p>管理已创建的模型</p>
        <span class="module-status finished">已实现</span>
      </div>

      <div class="module-card glass-panel" @click="goToSimulation">
        <div class="module-icon simulation">
          <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <path d="M21 12a9 9 0 0 1-9 9m9-9a9 9 0 0 0-9-9m9 9H3m9 9a9 9 0 0 1-9-9m9 9c1.66 0 3-4 3-9s-1.34-9-3-9m0 18c-1.66 0-3-4-3-9s1.34-9 3-9"/>
          </svg>
        </div>
        <h3>仿真运行</h3>
        <p>运行模型仿真</p>
        <span class="module-status finished">已实现</span>
      </div>

      <div class="module-card glass-panel" @click="goToNodeMonitor">
        <div class="module-icon node-monitor">
          <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
            <rect x="2" y="2" width="20" height="20" rx="2"/>
            <path d="M8 2V6M16 2V6M4 10H20"/>
            <circle cx="12" cy="14" r="2"/>
          </svg>
        </div>
        <h3>节点监控</h3>
        <p>监控集群节点状态</p>
        <span class="module-status finished">已实现</span>
      </div>
    </div>

    <!-- 用户管理弹窗 -->
    <UserManagement 
      v-if="showUserManagement" 
      @close="showUserManagement = false"
      @user-changed="refreshUsers"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { useUserStore } from '../stores/user'
import UserManagement from './UserManageMent.vue'

const router = useRouter()
const userStore = useUserStore()
const showUserManagement = ref(false)

// 直接使用 userStore，不需要 currentUser computed
// 在模板中直接使用 userStore.userInfo

const handleLogout = () => {
  userStore.logout()
  router.push('/login')
}

const goToModelDesign = () => {
  router.push('/projects')
}

const goToModelManage = () => { 
  router.push('/model-manage')
}

const goToSimulation = () => {
  router.push('/simulation')
}

const goToNodeMonitor = () => {
  router.push('/node-monitor')
}

const refreshUsers = () => {
  // 刷新用户列表
}

onMounted(() => {
  // 加载用户信息
  userStore.loadUserFromStorage()
  console.log('User info:', userStore.userInfo)
})
</script>

<style scoped>
.home-container {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(135deg, #0f172a 0%, #1e293b 100%);
  font-family: 'Inter', sans-serif;
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

.btn-user, .btn-logout {
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

.btn-user:hover {
  background: rgba(59, 130, 246, 0.2);
  border-color: rgba(59, 130, 246, 0.4);
  color: #3b82f6;
}

.btn-logout:hover {
  background: rgba(239, 68, 68, 0.2);
  border-color: rgba(239, 68, 68, 0.4);
  color: #ef4444;
}

.modules-container {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 24px;
  padding: 40px 24px;
  position: relative;
  z-index: 10;
}

.module-card {
  padding: 32px 24px;
  text-align: center;
  cursor: pointer;
  transition: all 0.3s ease;
}

.module-card:hover {
  transform: translateY(-8px);
  border-color: rgba(59, 130, 246, 0.4);
  box-shadow: 0 20px 40px -12px rgba(0, 0, 0, 0.3);
}

.module-icon {
  width: 80px;
  height: 80px;
  margin: 0 auto 20px;
  border-radius: 24px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.module-icon.model-design {
  background: linear-gradient(135deg, rgba(16, 185, 129, 0.2), rgba(16, 185, 129, 0.1));
  color: #10b981;
}

.module-icon.model-manage {
  background: linear-gradient(135deg, rgba(139, 92, 246, 0.2), rgba(139, 92, 246, 0.1));
  color: #8b5cf6;
}

.module-icon.simulation {
  background: linear-gradient(135deg, rgba(245, 158, 11, 0.2), rgba(245, 158, 11, 0.1));
  color: #f59e0b;
}

.module-icon.node-monitor {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.2), rgba(59, 130, 246, 0.1));
  color: #3b82f6;
}

.module-card h3 {
  font-size: 20px;
  font-weight: 600;
  color: white;
  margin-bottom: 8px;
}

.module-card p {
  font-size: 13px;
  color: #94a3b8;
  margin-bottom: 16px;
}

.module-status {
  display: inline-block;
  padding: 4px 12px;
  border-radius: 20px;
  font-size: 11px;
  font-weight: 500;
}

.module-status.finished {
  background: rgba(192, 199, 209, 0.2);
  color: #09c519;
}

.module-status.coming {
  background: rgba(192, 199, 209, 0.2);
  color: #889189;
}

@media (max-width: 1024px) {
  .modules-container {
    grid-template-columns: repeat(2, 1fr);
  }
}

@media (max-width: 640px) {
  .modules-container {
    grid-template-columns: 1fr;
  }
}
</style>