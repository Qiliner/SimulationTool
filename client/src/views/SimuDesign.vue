<template>
  <div class="simulation-designer-container">
      <!-- 顶部工程信息栏 -->
      <div class="designer-header-bar">
        <button @click="backToProjectList" class="back-btn">← 返回工程列表</button>
        <div class="project-context">
          <span class="project-name">{{ curProject.selectedProject?.name }}</span>
          <span v-if="curProject.isReadOnly" class="readonly-badge">只读模式</span>
          <span v-if="curProject.currentInstance" class="instance-badge">实例: {{ curProject.currentInstance?.id }}</span>
        </div>
        <div class="lock-status" v-if="curProject.selectedProject?.lockedBy && curProject.selectedProject?.lockedBy === (userStore.userInfo?.username || 'admin')">
          <span class="lock-icon">🔒</span> 您正在编辑此工程
        </div>
        <div class="lock-status" v-else-if="curProject.selectedProject?.lockedBy">
          <span class="lock-icon">🔒</span> {{ curProject.selectedProject?.lockedBy }} 正在编辑
        </div>
      </div>

      <div class="app-container">
        <!-- 左侧面板 -->
        <el-splitter>
            <el-splitter-panel size="20%" min="250">
                <!-- <div class="left-panel"> -->
                    <!-- 左上：文件树 -->
                    <el-splitter layout="vertical">
                        <el-splitter-panel size="67%">
                            <div class="tree-panel">
                                <FileTree @node-drag-start="handleNodeDragStart" />
                            </div>
                        </el-splitter-panel>
                        <el-splitter-panel size="33%">
                            <div class="ip-panel">
                                <IpNodeList />
                            </div>
                        </el-splitter-panel> 
                    </el-splitter>
                <!-- </div> -->
            </el-splitter-panel>        
            <el-splitter-panel>
                <!-- 中间画布 -->
                <el-splitter layout="vertical">
                  <el-splitter-panel size="67%">
                      <div class="center-panel">
                        <CenterCanvas
                            ref="centerCanvasRef"
                            @node-selected="handleNodeSelected"
                        />
                    </div>
                  </el-splitter-panel>
                  <el-splitter-panel size="33%" min="200">
                      <div class="center-log">
                        <Log />
                    </div>
                  </el-splitter-panel> 
              </el-splitter>
            </el-splitter-panel> 
            <el-splitter-panel  size="20%" min="250">
                <!-- 右侧面板 -->
                <div class="right-panel">
                    <div class="events-panel">
                        <EventList />
                    </div>
                </div>
            </el-splitter-panel> 
        </el-splitter>
        
        <el-drawer
          v-model="drawerVisible"
          direction="rtl"
          title="初始化参数配置"
          :append-to-body="true"
          @close="drawerVisible = false"
          size="600"
        >
          <!-- <MembersConfig :selected-node="selectedNode" /> -->
           <Test></Test>
        </el-drawer>
      </div>
  </div>

</template>

<script setup lang="ts">
import { ref } from 'vue'
import FileTree from '@/components/FileTree.vue'
import IpNodeList from '@/components/IpNodeList.vue'
import CenterCanvas from '@/components/CenterCanvas.vue'
import MembersConfig from '@/components/MembersConfig.vue'
import EventList from '@/components/EventList.vue'
import Log from '@/components/Log.vue'
import type { Node } from '@vue-flow/core'
import { ElDrawer, ElSplitter,ElSplitterPanel} from 'element-plus'
import { useRouter,useRoute} from 'vue-router'
import { useCurProjectStore } from '@/stores/curProjectInfo'
import { useUserStore } from '@/stores/user'
import 'element-plus/dist/index.css'
import '@vue-flow/core/dist/style.css'
import '@vue-flow/controls/dist/style.css'
import '@vue-flow/minimap/dist/style.css'

import Test from '@/components/Test.vue'

const centerCanvasRef = ref<InstanceType<typeof CenterCanvas> | null>(null)
const selectedNode = ref<Node | null>(null)
const drawerVisible = ref(false)
const curProject = useCurProjectStore()
const userStore = useUserStore()
const router = useRouter()
const route = useRoute()

console.log("route params",route.params)

const backToProjectList = ()=>{
  router.push("/simulation")
}

// 处理从文件树拖拽开始的节点数据
const handleNodeDragStart = (modelData: any) => {
  // 通过自定义事件存储拖拽数据
  if (centerCanvasRef.value) {
    centerCanvasRef.value.setDragModelData(modelData)
  }
}

// 处理画布中节点选中事件
const handleNodeSelected = (node: Node) => {
  drawerVisible.value = true
  selectedNode.value = node
}
</script>

<style scoped>
.app-container {
  display: flex;
  width: 100vw;
  height: 100vh;
  overflow: hidden;
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

.left-panel {
  width: 260px;
  height: 100%;
  display: flex;
  flex-direction: column;
  border-right: 1px solid #e4e7ed;
  background-color: #f5f7fa;
}

.tree-panel {
    height: 100%;
    flex: 1;
    overflow: auto;
    padding: 12px;
     background-color: #f5f7fa;
}

.ip-panel {
    height: 100%;
    flex: 1;
    overflow: auto;
    padding: 12px; 
     background-color: #f5f7fa;
}

.center-panel {
    height: 100%;
  flex-grow: 2;
  position: relative;
  padding: 12px 0;
  background-color: #f5f7fa;
}

.center-log {
  height: 100%;
  flex-grow: 1;
  position: relative;
  padding: 12px 0;
  background-color: #f5f7fa;
}

.right-panel {
    height: 100%;
  display: flex;
  flex-direction: column;
  border-left: 1px solid #e4e7ed;
  background-color: #f5f7fa;
}

.events-panel {
  flex: 1;
  overflow: auto;
  padding: 12px;
}

.simulation-designer-container {
  display: flex;
  flex-direction: column;
  height: 100vh;
  position: relative;
  z-index: 10;
  background: transparent;
}
.designer-header-bar {
  display: flex;
  align-items: center;
  gap: 20px;
  padding: 12px 24px;
  background: rgba(15, 25, 35, 0.8);
  backdrop-filter: blur(20px);
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
  flex-shrink: 0;
}
.back-btn {
  padding: 6px 16px;
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  color: #cbd5e1;
  cursor: pointer;
  transition: all 0.2s;
}
.back-btn:hover {
  background: rgba(255, 255, 255, 0.15);
}
.project-context {
  flex: 1;
}
.project-name {
  font-weight: bold;
  font-size: 16px;
  color: white;
  margin-right: 12px;
}
.readonly-badge {
  background: rgba(245, 158, 11, 0.2);
  color: #fbbf24;
  padding: 2px 8px;
  border-radius: 20px;
  font-size: 11px;
  margin-right: 8px;
}
.instance-badge {
  background: rgba(16, 185, 129, 0.2);
  color: #10b981;
  padding: 2px 8px;
  border-radius: 20px;
  font-size: 11px;
}
.lock-status {
  font-size: 12px;
  color: #94a3b8;
}
.lock-icon {
  margin-right: 4px;
}

:deep(.el-button){
  margin-left: 2px;
}

</style>