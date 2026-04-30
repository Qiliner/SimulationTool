<template>
  <div class="modal-overlay" @click.self="handleClose">
    <div class="modal-content glass-panel">
      <div class="modal-header">
        <h3>👥 用户管理</h3>
        <button class="modal-close" @click="handleClose">×</button>
      </div>

      <div class="modal-body">
        <div class="user-toolbar">
          <button class="btn-add" @click="openCreateDialog" v-if="canManageUsers">
            + 新建用户
          </button>
        </div>

        <div class="user-table-wrapper">
          <table class="user-table">
            <thead>
              <tr>
                <th>ID</th>
                <th>用户名</th>
                <th>邮箱</th>
                <th>角色</th>
                <th>状态</th>
                <th>创建时间</th>
                <th>最后登录</th>
                <th v-if="canManageUsers">操作</th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="user in users" :key="user.id">
                <td>{{ user.id }}</td>
                <td>{{ user.username }}</td>
                <td>{{ user.email || '-' }}</td>
                <td>
                  <span :class="['role-badge', getRoleClass(user.role)]">
                    {{ user.roleName }}
                  </span>
                </td>
                <td>
                  <span :class="['status-badge', user.isActive ? 'active' : 'inactive']">
                    {{ user.isActive ? '激活' : '禁用' }}
                  </span>
                </td>
                <td>{{ formatDate(user.createdAt) }}</td>
                <td>{{ formatDate(user.lastLogin) }}</td>
                <td v-if="canManageUsers">
                  <div class="table-actions">
                    <button class="btn-edit" @click="openEditDialog(user)" title="编辑">✎</button>
                    <button class="btn-delete" @click="deleteUser(user)" title="删除" :disabled="user.id === currentUser?.id">×</button>
                  </div>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <div class="modal-actions">
        <button class="btn-secondary" @click="handleClose">关闭</button>
      </div>
    </div>

    <!-- 编辑/创建用户弹窗 -->
    <div v-if="showUserDialog" class="sub-modal-overlay" @click.self="closeUserDialog">
      <div class="sub-modal-content glass-panel">
        <div class="modal-header">
          <h3>{{ isEditing ? '编辑用户' : '新建用户' }}</h3>
          <button class="modal-close" @click="closeUserDialog">×</button>
        </div>
        <div class="modal-body">
          <div class="form-group">
            <label>用户名 *</label>
            <input v-model="editingUser.username" class="form-input" placeholder="请输入用户名" />
          </div>
          <div class="form-group">
            <label>密码 {{ isEditing ? '(留空则不修改)' : '*' }}</label>
            <input v-model="editingUser.password" type="password" class="form-input" placeholder="请输入密码" />
          </div>
          <div class="form-group">
            <label>邮箱</label>
            <input v-model="editingUser.email" class="form-input" placeholder="请输入邮箱" />
          </div>
          <div class="form-group">
            <label>角色</label>
            <select v-model="editingUser.role" class="form-select">
              <option :value="0">超级管理员</option>
              <option :value="1">管理员</option>
              <option :value="2">设计者</option>
              <option :value="3">观察者</option>
            </select>
          </div>
          <div class="form-group">
            <label>
              <input type="checkbox" v-model="editingUser.isActive" />
              激活用户
            </label>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="saveUser" :disabled="isLoading">
            {{ isLoading ? '保存中...' : '保存' }}
          </button>
          <button class="btn-secondary" @click="closeUserDialog">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, computed } from 'vue'
import { useUserStore } from '../stores/user'

const emit = defineEmits(['close', 'user-changed'])

const userStore = useUserStore()
const API_BASE_URL = 'http://192.168.156.20:8080/api'
const users = ref<any[]>([])
const isLoading = ref(false)
const showUserDialog = ref(false)
const isEditing = ref(false)
const editingUser = ref({
  id: null,
  username: '',
  password: '',
  email: '',
  role: 3,
  isActive: true
})

// 使用 store 中的用户信息
const currentUser = computed(() => userStore.userInfo)

const canManageUsers = computed(() => {
  return userStore.isSuperAdmin || userStore.isAdmin
})

const getHeaders = () => {
  return {
    'Content-Type': 'application/json',
    'Authorization': `Bearer ${userStore.token}`
  }
}

const fetchUsers = async () => {
  if (!canManageUsers.value) return
  
  try {
    const response = await fetch(`${API_BASE_URL}/users`, {
      headers: getHeaders()
    })
    if (response.ok) {
      users.value = await response.json()
    }
  } catch (error) {
    console.error('Failed to fetch users:', error)
  }
}

const formatDate = (timestamp: string) => {
  if (!timestamp || timestamp === '0') return '-'
  const date = new Date(parseInt(timestamp) * 1000)
  return date.toLocaleDateString('zh-CN')
}

const getRoleClass = (role: number) => {
  switch (role) {
    case 0: return 'super-admin'
    case 1: return 'admin'
    case 2: return 'designer'
    default: return 'viewer'
  }
}

const openCreateDialog = () => {
  isEditing.value = false
  editingUser.value = {
    id: null,
    username: '',
    password: '',
    email: '',
    role: 3,
    isActive: true
  }
  showUserDialog.value = true
}

const openEditDialog = (user: any) => {
  isEditing.value = true
  editingUser.value = {
    id: user.id,
    username: user.username,
    password: '',
    email: user.email || '',
    role: user.role,
    isActive: user.isActive
  }
  showUserDialog.value = true
}

const saveUser = async () => {
  if (!editingUser.value.username) {
    alert('请输入用户名')
    return
  }

  isLoading.value = true

  try {
    const url = isEditing.value 
      ? `${API_BASE_URL}/users/${editingUser.value.id}`
      : `${API_BASE_URL}/users`
    const method = isEditing.value ? 'PUT' : 'POST'
    
    const data: any = {
      username: editingUser.value.username,
      email: editingUser.value.email,
      role: editingUser.value.role,
      isActive: editingUser.value.isActive
    }
    
    if (editingUser.value.password) {
      data.password = editingUser.value.password
    }

    const response = await fetch(url, {
      method,
      headers: getHeaders(),
      body: JSON.stringify(data)
    })

    if (response.ok) {
      alert(isEditing.value ? '用户更新成功' : '用户创建成功')
      closeUserDialog()
      fetchUsers()
      emit('user-changed')
    } else {
      const error = await response.json()
      alert(error.error || '操作失败')
    }
  } catch (error) {
    console.error('Failed to save user:', error)
    alert('操作失败')
  } finally {
    isLoading.value = false
  }
}

const deleteUser = async (user: any) => {
  if (!confirm(`确定要删除用户 "${user.username}" 吗？`)) return
  
  // 不能删除自己
  if (currentUser.value && user.id === currentUser.value.id) {
    alert('不能删除当前登录的用户')
    return
  }
  
  try {
    const response = await fetch(`${API_BASE_URL}/users/${user.id}`, {
      method: 'DELETE',
      headers: getHeaders()
    })
    
    if (response.ok) {
      alert('用户删除成功')
      fetchUsers()
      emit('user-changed')
    } else {
      const error = await response.json()
      alert(error.error || '删除失败')
    }
  } catch (error) {
    console.error('Failed to delete user:', error)
    alert('删除失败')
  }
}

const handleClose = () => {
  emit('close')
}

const closeUserDialog = () => {
  showUserDialog.value = false
}

onMounted(() => {
  fetchUsers()
})
</script>

<style scoped>
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
  width: 900px;
  max-width: 90%;
  max-height: 85vh;
  overflow: hidden;
  display: flex;
  flex-direction: column;
}

.sub-modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1100;
}

.sub-modal-content {
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

.user-toolbar {
  display: flex;
  justify-content: flex-end;
  margin-bottom: 20px;
}

.btn-add {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  padding: 8px 16px;
  border-radius: 20px;
  color: white;
  font-size: 13px;
  cursor: pointer;
}

.user-table-wrapper {
  overflow-x: auto;
}

.user-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 13px;
}

.user-table th,
.user-table td {
  padding: 12px 12px;
  text-align: left;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}

.user-table th {
  color: #94a3b8;
  font-weight: 500;
  background: rgba(0, 0, 0, 0.2);
}

.user-table td {
  color: #e2e8f0;
}

.role-badge {
  display: inline-block;
  padding: 2px 8px;
  border-radius: 12px;
  font-size: 11px;
}

.role-badge.super-admin {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.role-badge.admin {
  background: rgba(245, 158, 11, 0.2);
  color: #f59e0b;
}

.role-badge.designer {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.role-badge.viewer {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.status-badge {
  display: inline-block;
  padding: 2px 8px;
  border-radius: 12px;
  font-size: 11px;
}

.status-badge.active {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
}

.status-badge.inactive {
  background: rgba(100, 116, 139, 0.2);
  color: #94a3b8;
}

.table-actions {
  display: flex;
  gap: 8px;
}

.btn-edit, .btn-delete {
  width: 28px;
  height: 28px;
  border-radius: 8px;
  border: none;
  cursor: pointer;
  font-size: 14px;
}

.btn-edit {
  background: rgba(59, 130, 246, 0.2);
  color: #3b82f6;
}

.btn-edit:hover {
  background: rgba(59, 130, 246, 0.4);
}

.btn-delete {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.btn-delete:hover:not(:disabled) {
  background: rgba(239, 68, 68, 0.4);
}

.btn-delete:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.modal-actions {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  padding: 16px 24px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
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

.form-input, .form-select {
  width: 100%;
  padding: 10px 14px;
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 12px;
  color: white;
  font-size: 14px;
}

.form-input:focus, .form-select:focus {
  outline: none;
  border-color: #3b82f6;
}
</style>