import axios, { type AxiosInstance, type InternalAxiosRequestConfig, type AxiosResponse } from 'axios'
import { useUserStore } from '@/stores/user'

export const WS_BASE_URL = 'ws://192.168.156.20:8088'

export const API_BASE_URL = 'http://192.168.156.20:8080/api'

// 创建axios实例
const request: AxiosInstance = axios.create({
  baseURL: API_BASE_URL,
  timeout: 10000, // 请求超时时间
  headers: {
    'Content-Type': 'application/json'
  }
})

// 请求拦截器
request.interceptors.request.use(
  (config: InternalAxiosRequestConfig) => {
    const userStore = useUserStore()
    if (userStore.token) {
      config.headers.Authorization = `Bearer ${userStore.token}`
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// 响应拦截器
request.interceptors.response.use(
  (response: AxiosResponse) => {
    return response.data
  },
  (error) => {
    if (error.response?.status === 401) {
      // 未授权，清除token并跳转到登录页
      const userStore = useUserStore()
      userStore.logout()
      // 这里可以添加跳转逻辑，比如使用router.push('/login')
    }
    return Promise.reject(error)
  }
)

export { request }