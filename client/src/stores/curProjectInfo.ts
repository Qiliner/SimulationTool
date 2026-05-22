import { defineStore } from 'pinia'
import { ref, computed, watch } from 'vue'

export const useCurProjectStore = defineStore('curProjectInfo', () => {
  // ============ State ============
  const selectedProject = ref<any>(null)
  const currentInstance = ref<any>(null)
  const isReadOnly = ref(false)
  const projects = ref<any>([])
  const projectTab = ref<'public' | 'private'>('public')
  const projectSearch = ref('')

  // ============ Computed ============
  const filteredProjects = computed(() => {
    let list = projects.value.filter((p: any) => p.visibility === projectTab.value)
    if (projectSearch.value) {
      const kw = projectSearch.value.toLowerCase()
      list = list.filter((p: any) => p.name.toLowerCase().includes(kw))
    }
    return list
  })

  // ============ Persistence ============
  const persistState = () => {
    try {
      localStorage.setItem('curProjectState', JSON.stringify({
        selectedProject: selectedProject.value,
        currentInstance: currentInstance.value,
        isReadOnly: isReadOnly.value
      }))
    } catch (e) {
      console.error('Failed to persist curProject state:', e)
    }
  }

  const restoreState = () => {
    try {
      const stored = localStorage.getItem('curProjectState')
      if (stored) {
        const state = JSON.parse(stored)
        selectedProject.value = state.selectedProject
        currentInstance.value = state.currentInstance
        isReadOnly.value = state.isReadOnly
      }
    } catch (e) {
      console.error('Failed to restore curProject state:', e)
    }
  }

  // Watch changes and persist
  watch([selectedProject, currentInstance, isReadOnly], persistState, { deep: true })

  // Restore on init
  restoreState()

  return {
    selectedProject,
    currentInstance,
    isReadOnly,
    projects,
    projectTab,
    projectSearch,
    filteredProjects
  }
})
