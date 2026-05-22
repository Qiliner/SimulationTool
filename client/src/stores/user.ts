// src/stores/user.ts
import { defineStore } from 'pinia'
import { ref, computed } from 'vue'

export interface UserInfo {
  id: number
  username: string
  email: string
  role: number
  roleName: string
  isActive: boolean
}

export const useUserStore = defineStore('user', () => {
  // 状态
  const token = ref<string>(localStorage.getItem('token') || '')
  const userInfo = ref<UserInfo | null>(null)

  // 计算属性
  const isLoggedIn = computed(() => !!token.value)
  const isSuperAdmin = computed(() => userInfo.value?.role === 0)
  const isAdmin = computed(() => userInfo.value?.role === 0 || userInfo.value?.role === 1)
  const isDesigner = computed(() => userInfo.value?.role === 0 || userInfo.value?.role === 1 || userInfo.value?.role === 2)
  const currentRole = computed(() => userInfo.value?.role || 3)
  const currentRoleName = computed(() => userInfo.value?.roleName || '观察者')
  const roleClass = computed(() => {
    switch (userInfo.value?.role) {
      case 0: return 'super-admin'
      case 1: return 'admin'
      case 2: return 'designer'
      default: return 'viewer'
    } })

  // 方法
  const setUser = (user: UserInfo, userToken: string) => {
    userInfo.value = user
    token.value = userToken
    localStorage.setItem('token', userToken)
    localStorage.setItem('user', JSON.stringify(user))
  }

  const updateUser = (user: UserInfo) => {
    userInfo.value = user
    localStorage.setItem('user', JSON.stringify(user))
  }

  const logout = () => {
    userInfo.value = null
    token.value = ''
    localStorage.removeItem('token')
    localStorage.removeItem('user')
  }

  const loadUserFromStorage = () => {
    const storedToken = localStorage.getItem('token')
    const storedUser = localStorage.getItem('user')
    if (storedToken && storedUser && storedUser !== 'undefined') {
      token.value = storedToken
      try {
        userInfo.value = JSON.parse(storedUser)
      } catch (e) {
        console.error('Failed to parse stored user:', e)
        logout()
      }
    } else if (storedToken) {
      console.warn('Stored user data is invalid or missing. Clearing authentication state.')
      logout()
    }
  }

  const getAuthHeader = () => {
    return {
      'Content-Type': 'application/json',
      'Authorization': token.value ? `Bearer ${token.value}` : ''
    }
  }

  return {
    // 状态
    token,
    userInfo,
    // 计算属性
    isLoggedIn,
    isSuperAdmin,
    isAdmin,
    isDesigner,
    currentRole,
    currentRoleName,
    roleClass,
    // 方法
    setUser,
    updateUser,
    logout,
    loadUserFromStorage,
    getAuthHeader
  }
})