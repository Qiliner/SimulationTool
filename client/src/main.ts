// import './assets/main.css'

// import { createApp } from 'vue'
// import ModelDesigner from './ModelDesigner.vue'
// import App from './App.vue'
// createApp(ModelDesigner).mount('#app')
import { createApp } from 'vue'
import { createPinia } from 'pinia'
import router from './router'
import App from './App.vue'
import { useUserStore } from './stores/user'

// 引入样式
// import './style.css'

const app = createApp(App)

app.use(createPinia())
app.use(router)

// 在应用启动时加载用户状态
const userStore = useUserStore()
userStore.loadUserFromStorage()

app.mount('#app')