import { createRouter, createWebHistory } from 'vue-router'
import { useUserStore } from '../stores/user'
import Login from '../views/Login.vue'
import Home from '../views/Home.vue'
import ProjectList from '../views/ProjectList.vue'
import ModelDesigner from '../views/ModelDesigner.vue'
import NodeMonitor  from '../views/NodeMonitor.vue'
import ModelManage from '../views/ModelManage.vue'
import SimulationRun from '../views/SimulationRun.vue' 

const routes = [
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: { requiresAuth: false }
  },
  {
    path: '/',
    name: 'Home',
    component: Home,
    meta: { requiresAuth: true }
  },
  {
    path: '/projects',
    name: 'ProjectList',
    component: ProjectList,
    meta: { requiresAuth: true }
  },
  {
    path: '/model-designer',
    name: 'ModelDesigner',
    component: ModelDesigner,
    meta: { requiresAuth: true }
  },
  {
    path: '/node-monitor',
    name: 'NodeMonitor',
    component: NodeMonitor,
    meta: { requiresAuth: true }
  },
  {
  path: '/model-manage',
  name: 'ModelManage',
  component: ModelManage,
  meta: { requiresAuth: true }
  },
  {
    path: '/simulation',  // 新增仿真运行路由
    name: 'SimulationRun',
    component: SimulationRun,
    meta: { requiresAuth: true }
  }

]

const router = createRouter({
  history: createWebHistory(),
  routes
})

router.beforeEach((to, from) => {
  const userStore = useUserStore()
  
  if (to.meta.requiresAuth && !userStore.isLoggedIn) {
    return '/login'
  }
  
  if (to.path === '/login' && userStore.isLoggedIn) {
    return '/'
  }
  
  return true
})

export default router