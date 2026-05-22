import { defineStore } from "pinia";
import { ref,computed } from "vue";


export interface LogInfo {
  message: string,
  type: 'info' | 'warning' | 'error',
  timestamp: string
}

export const useLogStore = defineStore('log', () => {
    const logs = ref<LogInfo[]>([])
    const addLog = (message: string, type: 'info' | 'warning' | 'error' = 'info') => {
        logs.value.push({
        message,
        type,
        timestamp: new Date().toISOString()
    })
    }
    const clearLogs = () => {
        logs.value = []
    }
    // const errorLogs = computed(() => logs.value.filter(log => log.type === 'error'))
    // const warningLogs = computed(() => logs.value.filter(log => log.type === 'warning'))
    // const infoLogs = computed(() => logs.value.filter(log => log.type === 'info'))
    return {
        logs,
        addLog,
        clearLogs
        // errorLogs,
        // warningLogs,
        // infoLogs
  }
})