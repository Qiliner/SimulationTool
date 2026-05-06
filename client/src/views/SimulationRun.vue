<!-- SimulationRun.vue (修正版) -->
<template>
  <div class="simulation-run-container">
    <!-- 背景装饰 -->
    <div class="bg-decoration">
      <div class="gradient-sphere sphere-1"></div>
      <div class="gradient-sphere sphere-2"></div>
      <div class="gradient-sphere sphere-3"></div>
    </div>

    <!-- 工程选择界面 -->
    <div v-if="!selectedProject" class="project-selection-full">
      <!-- 顶部导航栏 -->
      <div class="navbar glass-panel">
        <div class="nav-left">
          <div class="logo-icon" @click="goHome">
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
          <span class="nav-title">仿真运行 - 选择工程</span>
        </div>
        <div class="nav-right">
          <div class="user-info" v-if="userStore && userStore.userInfo">
            <span class="user-name">{{ userStore.userInfo.username }}</span>
            <span class="user-role" :class="roleClass">{{ userStore.userInfo.roleName }}</span>
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

      <!-- 主要内容区 -->
      <div class="projects-container">
        <!-- 标签页 + 搜索 + 新建按钮 -->
        <div class="projects-header">
          <div class="tabs-wrapper">
            <button 
              :class="['tab-btn', { active: projectTab === 'public' }]"
              @click="projectTab = 'public'"
            >
              公开工程
            </button>
            <button 
              :class="['tab-btn', { active: projectTab === 'private' }]"
              @click="projectTab = 'private'"
            >
              私有工程
            </button>
          </div>
          <div class="search-wrapper">
            <input
              type="text"
              class="search-input"
              v-model="projectSearch"
              placeholder="🔍 搜索工程名称..."
            />
            <button @click="showCreateProject = true" class="btn-primary">+ 新建工程</button>
            <button @click="refreshProjects" class="btn-secondary">刷新</button>
          </div>
        </div>

        <!-- 工程卡片网格 -->
       <div class="projects-grid">
        <div 
          v-for="project in filteredProjects" 
          :key="project.id"
          class="project-card glass-panel"
        >
          <!-- 上方：工程信息（图标 + 详情） -->
          <div class="project-info-area">
            <div class="project-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <path d="M4 4H20V20H4V4Z"/>
                <path d="M8 7H16M8 12H16M8 17H12"/>
              </svg>
            </div>
            <div class="project-details">
              <h3>{{ project.name }}</h3>
              <p class="project-desc">{{ project.description || '无描述' }}</p>
              <div class="project-meta">
                <span class="project-owner">创建者: {{ project.creator }}</span>
                <span class="project-time">创建: {{ project.createTime }}</span>
                <span :class="['visibility-badge', project.visibility]">
                  {{ project.visibility === 'public' ? '公开' : '私有' }}
                </span>
              </div>
            </div>
          </div>

          <!-- 中部：运行实例列表（宽度拉长，横向或网格布局） -->
          <div v-if="project.runningInstances && project.runningInstances.length > 0" class="instance-list-wide">
            <div class="instances-title">运行实例</div>
            <div class="instance-items">
              <div 
                v-for="instance in project.runningInstances" 
                :key="instance.id"
                class="instance-item-wide"
              >
                <span class="instance-id-wide">{{ instance.id }}</span>
                <span class="instance-runner-wide">{{ instance.startedBy }}</span>
                <button @click="loadInstance(project, instance)" class="load-instance-wide">
                  加载
                </button>
              </div>
            </div>
          </div>

          <!-- 底部：进入工程按钮（全宽或宽按钮） -->
          <div class="project-bottom-action">
            <button 
              @click="enterProject(project)" 
              class="enter-btn-full"
              :disabled="project.lockedBy && project.lockedBy !== currentUser"
              :title="getLockTip(project)"
            >
              {{ project.lockedBy && project.lockedBy !== currentUser ? '只读进入' : '进入工程' }}
            </button>
          </div>
        </div>
      </div>

        <!-- 空状态 -->
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
    <div v-else class="simulation-designer-container">
      <!-- 顶部工程信息栏 -->
      <div class="designer-header-bar">
        <button @click="backToProjectList" class="back-btn">← 返回工程列表</button>
        <div class="project-context">
          <span class="project-name">{{ selectedProject.name }}</span>
          <span v-if="isReadOnly" class="readonly-badge">只读模式</span>
          <span v-if="currentInstance" class="instance-badge">实例: {{ currentInstance.id }}</span>
        </div>
        <div class="lock-status" v-if="selectedProject.lockedBy && selectedProject.lockedBy === currentUser">
          <span class="lock-icon">🔒</span> 您正在编辑此工程
        </div>
        <div class="lock-status" v-else-if="selectedProject.lockedBy">
          <span class="lock-icon">🔒</span> {{ selectedProject.lockedBy }} 正在编辑
        </div>
      </div>

      <!-- 仿真设计主界面（移除测试用例列表） -->
      <div class="container simulation-design">
        <!-- 左列 - 仅保留资源列表和节点列表 -->
        <div class="left-column">
          <!-- 测试资源列表 -->
          <div class="panel left-bottom-panel">
            <div class="panel-header">
              测试资源列表
              <button @click="flushResources" class="flush-btn">🔃</button>
            </div>
            <input
              type="text"
              class="search-box"
              v-model="resourceSearch"
              placeholder="搜索测试资源..."
            />
            <div class="panel-content" id="resourceList">
              <div
                v-for="resource in filteredResources"
                :key="resource.id"
                class="node-item"
                :class="{ 'draggable-disabled': isReadOnly }"
                draggable="!isReadOnly"
                @dragstart="isReadOnly ? null : dragStart($event, resource)"
              >
                <div
                  class="node-icon"
                  :style="{ backgroundColor: resource.color }"
                >
                  {{ resource.icon }}
                </div>
                <div class="node-details">
                  <div class="node-name">{{ resource.name }}</div>
                  <div class="node-type">{{ resource.type }}</div>
                </div>
              </div>
            </div>
          </div>

          <!-- 仿真节点列表 -->
          <div class="panel">
            <div class="panel-header">仿真节点列表</div>
            <input
              type="text"
              class="search-box"
              v-model="nodeSearch"
              placeholder="搜索仿真节点..."
            />
            <div class="panel-content" id="simulationNodeList">
              <div
                v-for="node in filteredNodes"
                :key="node.id"
                class="node-item"
              >
                <div
                  class="node-icon"
                  :style="{ backgroundColor: node.color }"
                >
                  N
                </div>
                <div class="node-details">
                  <div class="node-name">{{ node.ip }}</div>
                  <div :class="['node-type', node.status]">
                    {{ node.status === 'online' ? '在线' : '离线' }}
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        <!-- 中列：仿真设计画布 -->
        <div class="middle-column">
          <div class="design-panel">
            <div class="design-header">仿真设计界面</div>

            <!-- 仿真控制面板 -->
            <div class="sim-control-panel">
              <div class="control-group">
                <span class="control-label">事件设置:</span>
                <input
                  type="text"
                  class="control-input"
                  v-model="eventSetting"
                  placeholder="输入事件"
                  :disabled="isReadOnly"
                />
              </div>
              <div class="control-group">
                <span class="control-label">倍速:</span>
                <input
                  type="number"
                  class="control-input"
                  v-model="speed"
                  min="0.1"
                  max="10"
                  step="0.1"
                  :disabled="isReadOnly"
                />
              </div>
              <button class="control-btn" @click="initializeSim" :disabled="isReadOnly">初始化</button>
              <button class="control-btn start" @click="startSim" :disabled="isReadOnly">启动</button>
              <button class="control-btn pause" @click="pauseSim" :disabled="isReadOnly">暂停</button>
              <button class="control-btn" @click="resumeSim" :disabled="isReadOnly">恢复</button>
              <button class="control-btn stop" @click="stopSim" :disabled="isReadOnly">停止</button>
              <div class="sim-time-display">{{ simTime.toFixed(2) }} s</div>
              <div>
                <span :class="['sim-status', simStatus.icon]"></span>
                <span>{{ simStatus.text }}</span>
              </div>
            </div>

            <div
              class="design-canvas"
              id="design-canvas"
              @dragover.prevent
              @drop="isReadOnly ? null : dropResource"
            >
              <!-- 画布上的模型节点 -->
              <div
                v-for="node in canvasNodes"
                :key="node.id"
                class="design-node"
                :style="{ left: node.x + 'px', top: node.y + 'px' }"
                @mousedown="isReadOnly ? null : startDrag(node, $event)"
                @click="selectModel(node)"
              >
                <div class="design-node-header">
                  <div class="node-icon" :style="{ backgroundColor: node.color }">
                    {{ node.icon }}
                  </div>
                  {{ node.name }}
                </div>
                <div class="design-node-body">{{ node.type }}</div>
                <div class="node-ports">
                  <div class="input-ports">
                    <div class="node-port input-port"></div>
                  </div>
                  <div class="output-ports">
                    <div class="node-port output-port"></div>
                  </div>
                </div>
              </div>
              
              <!-- 连接线 SVG -->
              <svg
                id="connectionLayer"
                style="position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; z-index: 1;"
              >
                <defs>
                  <marker id="arrowhead" markerWidth="10" markerHeight="7" refX="9" refY="3.5" orient="auto">
                    <polygon points="0 0, 10 3.5, 0 7" fill="#007bff" />
                  </marker>
                </defs>
                <line
                  v-for="connection in connections"
                  :key="connection.id"
                  :x1="connection.x1"
                  :y1="connection.y1"
                  :x2="connection.x2"
                  :y2="connection.y2"
                  class="connection"
                  :class="{ selected: connection.selected }"
                  marker-end="url(#arrowhead)"
                />
              </svg>
            </div>
            <div class="design-tools">
              <button class="tool-btn" @click="clearCanvas" :disabled="isReadOnly">清空画布</button>
              <button class="tool-btn" @click="saveDesign" :disabled="isReadOnly">保存设计</button>
              <button class="tool-btn" @click="loadDesign" :disabled="isReadOnly">加载设计</button>
              <button class="tool-btn primary-btn" @click="runSimulation" :disabled="isReadOnly">
                运行仿真
              </button>
            </div>
          </div>

          <!-- 日志窗口 -->
          <div class="monitor-panel">
            <div class="monitor-header">
              日志窗口
              <button class="tool-btn" @click="clearLog">清空</button>
            </div>
            <div class="monitor-content" id="log-window">
              <div
                v-for="(log, index) in logs"
                :key="index"
                :class="['monitor-line', log.type]"
              >
                <span class="monitor-timestamp">[{{ log.timestamp }}]</span>
                {{ log.message }}
              </div>
            </div>
          </div>
        </div>

        <!-- 右列：参数和事件面板 -->
        <div class="right-column">
          <div class="panel">
            <div class="panel-header">模型参数窗口</div>
            <div class="panel-content" id="model-params">
              <div v-if="selectedModel">
                <h3>{{ selectedModel.name }}</h3>
                <div v-for="(param, key) in selectedModel.params" :key="key" class="param-item">
                  <label>{{ param.label }}:</label>
                  <input
                    type="text"
                    v-model="param.value"
                    class="control-input"
                    style="width: 100%; margin-bottom: 5px;"
                    :disabled="isReadOnly"
                  />
                </div>
              </div>
              <p v-else>选择模型以设置参数</p>
            </div>
          </div>
          <div class="panel">
            <div class="panel-header">时间列表</div>
            <div class="panel-content">
              <div id="time-events" class="controls">
                <button
                  v-for="event in timeEvents"
                  :key="event.id"
                  @click="triggerEvent(event)"
                  class="tool-btn"
                  :disabled="isReadOnly"
                >
                  {{ event.name }}
                </button>
              </div>
              <input
                type="text"
                placeholder="自定义事件"
                v-model="customEvent"
                class="control-input"
                style="width: 100%; margin-top: 10px;"
                :disabled="isReadOnly"
              />
              <button
                @click="sendCustomEvent"
                class="tool-btn primary-btn"
                style="width: 100%; margin-top: 5px;"
                :disabled="isReadOnly"
              >
                发送
              </button>
            </div>
          </div>
          <div class="monitor-panel">
            <div class="monitor-header">
              数据监控窗口
              <button class="tool-btn" @click="exportMonitor" :disabled="isReadOnly">导出</button>
            </div>
            <div class="monitor-content" id="data-monitor">
              <div
                v-for="(data, index) in monitorData"
                :key="index"
                :class="['monitor-line', data.type]"
              >
                <span class="monitor-timestamp">[{{ data.timestamp }}]</span>
                {{ data.message }}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>


    <!-- 新建工程弹窗 -->
    <div v-if="showCreateProject" class="modal-overlay" @click.self="showCreateProject = false">
      <div class="modal-content">
        <h3>新建工程</h3>
        <input v-model="newProjectName" placeholder="工程名称" class="modal-input" />
        <div class="modal-radio">
          <label><input type="radio" value="public" v-model="newProjectVisibility" /> 公开工程</label>
          <label><input type="radio" value="private" v-model="newProjectVisibility" /> 私有工程</label>
        </div>
        <div class="modal-actions">
          <button @click="createProject" class="btn-primary">创建</button>
          <button @click="showCreateProject = false" class="btn-secondary">取消</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios';
import { useUserStore } from '@/stores/user';

export default {
  name: 'SimulationRun',
  data() {
    return {
      selectedProject: null,
      currentInstance: null,
      isReadOnly: false,
      projectTab: 'public',
      showCreateProject: false,
      newProjectName: '',
      newProjectVisibility: 'public',
      projectSearch: '',
      projects: [],
      userStore: null,
      currentUser: '',
      models: [],
      resources: [],
      resourceSearch: '',
      nodes: [
        { id: 1, ip: '192.168.1.100', status: 'online', color: '#4CAF50' },
        { id: 2, ip: '192.168.1.101', status: 'offline', color: '#f44336' },
        { id: 3, ip: '192.168.1.102', status: 'online', color: '#4CAF50' },
        { id: 4, ip: '192.168.156.1', status: 'online', color: '#4CAF50' },
        { id: 5, ip: '192.168.156.2', status: 'offline', color: '#9E9E9E' },
      ],
      nodeSearch: '',
      eventSetting: '',
      speed: 1,
      simTime: 0,
      simStatus: { icon: 'status-idle', text: '空闲' },
      canvasNodes: [],
      connections: [],
      selectedModel: null,
      draggingNode: null,
      dragOffset: { x: 0, y: 0 },
      logs: [],
      timeEvents: [
        { id: 1, name: '事件1' },
        { id: 2, name: '事件2' },
        { id: 3, name: '事件3' }
      ],
      customEvent: '',
      monitorData: [],
      draggedResource: null,
      axios_api: null,
      simInterval: null,
      monitorInterval: null
    };
  },
  computed: {
    filteredResources() {
      if (!this.resourceSearch) return this.resources;
      return this.resources.filter(resource =>
        resource.name.toLowerCase().includes(this.resourceSearch.toLowerCase()) ||
        resource.type.toLowerCase().includes(this.resourceSearch.toLowerCase())
      );
    },
    filteredNodes() {
      if (!this.nodeSearch) return this.nodes;
      return this.nodes.filter(node => node.ip.includes(this.nodeSearch));
    },
    filteredProjects() {
      let list = this.projects.filter(p => p.visibility === this.projectTab);
      if (this.projectSearch) {
        const kw = this.projectSearch.toLowerCase();
        list = list.filter(p => p.name.toLowerCase().includes(kw));
      }
      return list;
    },
    roleClass() {
      if (!this.userStore?.userInfo) return '';
      switch (this.userStore.userInfo?.role) {
        case 0: return 'super-admin';
        case 1: return 'admin';
        case 2: return 'designer';
        default: return 'viewer';
      }
    }
  },
  mounted() {
    const userStore = useUserStore();
    this.userStore = userStore;
    this.currentUser = userStore.userInfo?.username || 'admin';
    this.logs = [{ timestamp: this.getCurrentTime(), message: '仿真系统初始化完成', type: 'info' }];
    this.monitorData = [{ timestamp: this.getCurrentTime(), message: '监控系统启动', type: 'info' }];
    this.initMockProjects();
    this.createAxios();
    this.flushResources();
    
    this.simInterval = setInterval(() => {
      if (this.simStatus.icon === 'status-running') {
        this.simTime += 0.1;
      }
    }, 100);
    
    this.monitorInterval = setInterval(() => {
      if (this.simStatus.icon === 'status-running') {
        this.monitorData.push({
          timestamp: this.getCurrentTime(),
          message: `仿真时间: ${this.simTime.toFixed(2)}s, 事件: ${this.eventSetting || '无'}`,
          type: 'info'
        });
        if (this.monitorData.length > 50) {
          this.monitorData.splice(0, this.monitorData.length - 50);
        }
      }
    }, 1000);
  },
  beforeUnmount() {
    if (this.simInterval) clearInterval(this.simInterval);
    if (this.monitorInterval) clearInterval(this.monitorInterval);
    if (this.selectedProject && this.selectedProject.lockedBy === this.currentUser) {
      this.releaseProjectLock(this.selectedProject);
    }
  },
  methods: {
    goHome() { this.$router.push('/'); },
    handleLogout() { this.userStore.logout(); this.$router.push('/login'); },
    getCurrentTime() {
      const now = new Date();
      return `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`;
    },
    initMockProjects() {
      this.projects = [
        {
          id: 1,
          name: '智能电网仿真工程',
          visibility: 'public',
          creator: 'zhangsan',
          createTime: '2024-01-15',
          lockedBy: null,
          lockedAt: null,
          runningInstances: [
            { id: 'inst_001', startedBy: 'lisi', startTime: '2024-01-20 10:30:00', status: 'running' }
          ],
          designData: null
        },
        {
          id: 2,
          name: '自动驾驶测试工程',
          visibility: 'public',
          creator: 'wangwu',
          createTime: '2024-02-10',
          lockedBy: null,
          lockedAt: null,
          runningInstances: [],
          designData: null
        },
        {
          id: 3,
          name: '工业机器人仿真',
          visibility: 'private',
          creator: this.currentUser,
          createTime: '2024-03-05',
          lockedBy: null,
          lockedAt: null,
          runningInstances: [
            { id: 'inst_003', startedBy: this.currentUser, startTime: '2024-03-10 14:20:00', status: 'running' }
          ],
          designData: null
        }
      ];
    },
    refreshProjects() { this.initMockProjects(); this.addLog('info', '刷新工程列表'); },
    createProject() {
      if (!this.newProjectName.trim()) { alert('请输入工程名称'); return; }
      const newId = Math.max(...this.projects.map(p => p.id), 0) + 1;
      const newProject = {
        id: newId,
        name: this.newProjectName,
        visibility: this.newProjectVisibility,
        creator: this.currentUser,
        createTime: new Date().toISOString().slice(0, 10),
        lockedBy: null,
        lockedAt: null,
        runningInstances: [],
        designData: null
      };
      this.projects.push(newProject);
      this.showCreateProject = false;
      this.newProjectName = '';
      this.addLog('info', `创建工程: ${newProject.name}`);
    },
    getLockTip(project) {
      if (project.lockedBy && project.lockedBy !== this.currentUser) {
        return `${project.lockedBy} 正在编辑此工程，将以只读模式打开`;
      }
      return '';
    },
    tryLockProject(project, readOnly = false) {
      if (readOnly) return false;
      if (project.lockedBy && project.lockedBy !== this.currentUser) {
        this.addLog('warning', `工程 ${project.name} 正被 ${project.lockedBy} 编辑，进入只读模式`);
        return false;
      }
      if (!project.lockedBy) {
        project.lockedBy = this.currentUser;
        project.lockedAt = new Date().toISOString();
        this.addLog('info', `获取工程 ${project.name} 编辑锁`);
        return true;
      }
      return false;
    },
    releaseProjectLock(project) {
      if (project.lockedBy === this.currentUser) {
        project.lockedBy = null;
        project.lockedAt = null;
        this.addLog('info', `释放工程 ${project.name} 编辑锁`);
      }
    },
    enterProject(project) {
      const canEdit = this.tryLockProject(project, false);
      this.selectedProject = { ...project };
      this.currentInstance = null;
      this.isReadOnly = !canEdit;
      this.loadProjectDesign();
      this.addLog('info', `${this.isReadOnly ? '只读进入' : '进入'}工程: ${project.name}`);
    },
    loadInstance(project, instance) {
      if (project.lockedBy && project.lockedBy !== this.currentUser) {
        this.isReadOnly = true;
      } else {
        this.isReadOnly = true;
      }
      this.selectedProject = { ...project };
      this.currentInstance = { ...instance };
      this.loadProjectDesign();
      this.addLog('info', `加载运行实例 ${instance.id}，进入只读监控模式`);
    },
    loadProjectDesign() {
      const saved = localStorage.getItem(`simulation_design_${this.selectedProject.id}`);
      if (saved) {
        const design = JSON.parse(saved);
        this.canvasNodes = design.nodes || [];
        this.connections = design.connections || [];
        this.addLog('info', `加载工程 "${this.selectedProject.name}" 的设计数据`);
      } else {
        this.canvasNodes = [];
        this.connections = [];
        this.addLog('info', `工程 "${this.selectedProject.name}" 无保存的设计，使用空画布`);
      }
    },
    saveProjectDesign() {
      const design = { nodes: this.canvasNodes, connections: this.connections };
      localStorage.setItem(`simulation_design_${this.selectedProject.id}`, JSON.stringify(design));
    },
    backToProjectList() {
      if (this.selectedProject && this.selectedProject.lockedBy === this.currentUser) {
        this.releaseProjectLock(this.selectedProject);
        const proj = this.projects.find(p => p.id === this.selectedProject.id);
        if (proj) { proj.lockedBy = null; proj.lockedAt = null; }
      }
      this.selectedProject = null;
      this.currentInstance = null;
      this.isReadOnly = false;
      this.canvasNodes = [];
      this.connections = [];
      this.selectedModel = null;
      this.simTime = 0;
      this.simStatus = { icon: 'status-idle', text: '空闲' };
      this.addLog('info', '返回工程列表');
    },
    createAxios() {
      this.axios_api = axios.create({ baseURL: 'http://192.168.156.20:8080', timeout: 10000 });
    },
    async flushResources() {
      try {
        this.addLog('info', '开始获取模型列表...');
        const response = await this.axios_api.get('/models');
        let models = response.data;
        if (!Array.isArray(models)) models = [];
        this.models = models;
        this.resources = models.map((v, idx) => ({
          id: idx + 1,
          name: v.name,
          type: v.type,
          color: '#9C27B0',
          icon: 'M'
        }));
        this.addLog('info', `成功获取 ${this.resources.length} 个模型`);
      } catch (error) {
        this.resources = [
          { id: 1, name: '模型A', type: '输入模型', color: '#4CAF50', icon: 'M' },
          { id: 2, name: '模型B', type: '处理模型', color: '#2196F3', icon: 'M' },
          { id: 3, name: '模型C', type: '输出模型', color: '#FF9800', icon: 'M' },
        ];
        this.addLog('warning', '获取模型失败，使用默认数据');
      }
    },
    dragStart(event, resource) { if (this.isReadOnly) return; this.draggedResource = resource; event.dataTransfer.setData('text/plain', JSON.stringify(resource)); },
    dropResource(event) { if (this.isReadOnly) return; event.preventDefault(); try { const resource = JSON.parse(event.dataTransfer.getData('text/plain')); if (!resource) return; const canvas = event.currentTarget; const rect = canvas.getBoundingClientRect(); const x = event.clientX - rect.left - 70; const y = event.clientY - rect.top - 50; this.canvasNodes.push({ id: Date.now(), ...resource, x: Math.max(0, Math.min(x, rect.width - 140)), y: Math.max(0, Math.min(y, rect.height - 100)) }); this.addLog('info', `添加模型到画布: ${resource.name}`); } catch (error) { console.error('拖拽错误:', error); } },
    startDrag(node, event) { if (this.isReadOnly) return; this.draggingNode = node; const rect = event.currentTarget.getBoundingClientRect(); this.dragOffset = { x: event.clientX - rect.left, y: event.clientY - rect.top }; document.addEventListener('mousemove', this.dragMove); document.addEventListener('mouseup', this.dragEnd); },
    dragMove(event) { if (!this.draggingNode || this.isReadOnly) return; const canvas = document.querySelector('.design-canvas'); const rect = canvas.getBoundingClientRect(); let x = event.clientX - rect.left - this.dragOffset.x; let y = event.clientY - rect.top - this.dragOffset.y; x = Math.max(0, Math.min(x, rect.width - 140)); y = Math.max(0, Math.min(y, rect.height - 100)); this.draggingNode.x = x; this.draggingNode.y = y; },
    dragEnd() { this.draggingNode = null; document.removeEventListener('mousemove', this.dragMove); document.removeEventListener('mouseup', this.dragEnd); },
    selectModel(node) { this.selectedModel = { name: node.name, params: { param1: { label: '参数1', value: node.param1 || '' }, param2: { label: '参数2', value: node.param2 || '' } } }; },
    clearCanvas() { if (this.isReadOnly) return; this.canvasNodes = []; this.connections = []; this.selectedModel = null; this.addLog('info', '清空画布'); },
    saveDesign() { if (this.isReadOnly) return; this.saveProjectDesign(); this.addLog('info', `设计已保存到工程 "${this.selectedProject.name}"`); },
    loadDesign() { this.loadProjectDesign(); },
    initializeSim() { if (this.isReadOnly) return; this.simTime = 0; this.simStatus = { icon: 'status-ready', text: '就绪' }; this.addLog('info', '仿真初始化完成'); },
    startSim() { if (this.isReadOnly) return; this.simStatus = { icon: 'status-running', text: '运行中' }; this.addLog('info', '仿真启动'); },
    pauseSim() { if (this.isReadOnly) return; this.simStatus = { icon: 'status-paused', text: '已暂停' }; this.addLog('info', '仿真暂停'); },
    resumeSim() { if (this.isReadOnly) return; this.simStatus = { icon: 'status-running', text: '运行中' }; this.addLog('info', '仿真恢复'); },
    stopSim() { if (this.isReadOnly) return; this.simStatus = { icon: 'status-idle', text: '空闲' }; this.addLog('info', '仿真停止'); },
    runSimulation() { if (this.isReadOnly) return; this.addLog('info', '开始运行仿真'); this.startSim(); },
    addLog(type, message) { this.logs.push({ timestamp: this.getCurrentTime(), message, type }); this.$nextTick(() => { const logWindow = document.getElementById('log-window'); if (logWindow) logWindow.scrollTop = logWindow.scrollHeight; }); },
    clearLog() { this.logs = []; },
    triggerEvent(event) { if (this.isReadOnly) return; this.addLog('event', `触发事件: ${event.name}`); },
    sendCustomEvent() { if (this.isReadOnly) return; if (this.customEvent.trim()) { this.addLog('event', `发送自定义事件: ${this.customEvent}`); this.customEvent = ''; } },
    exportMonitor() { this.addLog('info', '导出监控数据'); }
  }
};
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.simulation-run-container {
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
  z-index: 0;
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

/* ========== 顶部导航栏 ========== */
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
  cursor: pointer;
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
.btn-logout {
  display: flex;
  align-items: center;
  gap: 8px;
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  padding: 8px 16px;
  color: #cbd5e1;
  cursor: pointer;
  transition: all 0.2s;
}
.btn-logout:hover {
  background: rgba(239, 68, 68, 0.2);
  border-color: rgba(239, 68, 68, 0.4);
  color: #ef4444;
}

/* ========== 工程选择界面 ========== */
.project-selection-full {
  position: relative;
  z-index: 10;
  height: 100vh;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}
.projects-container {
  flex: 1;
  overflow-y: auto;
  padding: 24px;
}
.projects-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
  flex-wrap: wrap;
  gap: 16px;
}
.tabs-wrapper {
  display: flex;
  gap: 8px;
}
.tab-btn {
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
  background: rgba(59, 130, 246, 0.15);
  border: 1px solid rgba(59, 130, 246, 0.3);
}
.search-wrapper {
  display: flex;
  gap: 12px;
  align-items: center;
}
.search-input {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
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
.btn-primary, .btn-secondary {
  padding: 8px 20px;
  border-radius: 40px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  border: none;
}
.btn-primary {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  color: white;
}
.btn-secondary {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  color: #cbd5e1;
}
.btn-primary:hover, .btn-secondary:hover {
  transform: translateY(-1px);
}

/* 卡片网格 — 一行三个 */
.projects-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 20px;
}
.project-card {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  height: auto;
  min-height: 260px;
  padding: 20px;
  transition: all 0.3s ease;
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
}
.project-card:hover {
  transform: translateY(-4px);
  border-color: rgba(59, 130, 246, 0.4);
  box-shadow: 0 12px 32px rgba(0, 0, 0, 0.3);
}

/* 卡片内布局 */
.project-info-area {
  display: flex;
  gap: 16px;
  margin-bottom: 16px;
}
.project-icon {
  width: 48px;
  height: 48px;
  background: rgba(59, 130, 246, 0.15);
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #3b82f6;
  flex-shrink: 0;
}
.project-details {
  flex: 1;
  overflow: hidden;
}
.project-details h3 {
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
  flex-wrap: wrap;
  gap: 12px;
  font-size: 11px;
  color: #64748b;
}
.project-owner {
  color: #3b82f6;
}
.visibility-badge {
  padding: 2px 6px;
  border-radius: 12px;
  font-size: 10px;
}
.visibility-badge.public {
  background: rgba(16, 185, 129, 0.15);
  color: #10b981;
}
.visibility-badge.private {
  background: rgba(139, 92, 246, 0.15);
  color: #a78bfa;
}

/* 运行实例列表（横向宽区域） */
.instance-list-wide {
  background: rgba(0, 0, 0, 0.3);
  border-radius: 16px;
  padding: 12px;
  margin: 8px 0;
  width: 100%;
}
.instances-title {
  font-size: 12px;
  font-weight: 600;
  color: #cbd5e1;
  margin-bottom: 10px;
}
.instance-items {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
.instance-item-wide {
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
  padding: 8px 12px;
  width: 100%;
}
.instance-id-wide {
  font-family: monospace;
  color: #fbbf24;
  font-size: 12px;
  font-weight: 500;
}
.instance-runner-wide {
  color: #94a3b8;
  font-size: 11px;
  flex: 1;
  margin-left: 12px;
}
.load-instance-wide {
  background: rgba(245, 158, 11, 0.2);
  border: 1px solid rgba(245, 158, 11, 0.4);
  border-radius: 20px;
  color: #fbbf24;
  font-size: 11px;
  padding: 4px 12px;
  cursor: pointer;
  transition: all 0.2s;
}
.load-instance-wide:hover {
  background: rgba(245, 158, 11, 0.4);
}

/* 底部进入工程按钮（全宽） */
.project-bottom-action {
  margin-top: auto;
  width: 100%;
}
.enter-btn-full {
  width: 100%;
  padding: 10px 0;
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  border-radius: 40px;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}
.enter-btn-full:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
}
.enter-btn-full:disabled {
  background: rgba(100, 116, 139, 0.5);
  cursor: not-allowed;
}

.empty-state {
  text-align: center;
  padding: 80px;
  color: #64748b;
}
.empty-illustration svg {
  color: #334155;
  margin-bottom: 16px;
}

/* 响应式 */
@media (max-width: 1200px) {
  .projects-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}
@media (max-width: 768px) {
  .projects-grid {
    grid-template-columns: 1fr;
  }
}

/* ========== 仿真设计界面样式（保持原有） ========== */
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

.simulation-design {
  flex: 1;
  overflow: hidden;
  padding: 10px;
}
.container {
  display: grid;
  grid-template-columns: 280px 1fr 280px;
  height: 100%;
  gap: 12px;
  overflow: hidden;
}
.left-column, .middle-column, .right-column {
  display: flex;
  flex-direction: column;
  gap: 12px;
  height: 100%;
  overflow: hidden;
}
.panel {
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}
.panel-header {
  background: linear-gradient(135deg, rgba(59, 130, 246, 0.2), rgba(139, 92, 246, 0.2));
  padding: 12px 16px;
  font-weight: 600;
  font-size: 14px;
  color: white;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}
.panel-content {
  flex: 1;
  overflow-y: auto;
  padding: 12px;
}
.search-box {
  background: rgba(0, 0, 0, 0.3);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  padding: 8px 16px;
  margin: 12px;
  width: calc(100% - 24px);
  color: white;
  font-size: 13px;
}
.search-box::placeholder {
  color: #64748b;
}
.search-box:focus {
  outline: none;
  border-color: #3b82f6;
}
.node-item {
  padding: 10px;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
  display: flex;
  align-items: center;
  gap: 12px;
  cursor: grab;
  transition: background 0.2s;
}
.node-item:hover {
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
}
.draggable-disabled {
  cursor: not-allowed;
  opacity: 0.5;
}
.node-icon {
  width: 32px;
  height: 32px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: bold;
  color: white;
}
.node-details {
  flex: 1;
}
.node-name {
  font-weight: 500;
  color: white;
  font-size: 13px;
}
.node-type {
  font-size: 11px;
  color: #94a3b8;
}
.node-type.online { color: #10b981; }
.node-type.offline { color: #f87171; }

.design-panel {
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  flex: 1;
}
.design-header {
  background: linear-gradient(135deg, rgba(44, 62, 80, 0.6), rgba(74, 100, 145, 0.6));
  padding: 12px 16px;
  color: white;
  font-weight: 600;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}
.sim-control-panel {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px 16px;
  background: rgba(0, 0, 0, 0.2);
  flex-wrap: wrap;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
}
.control-group {
  display: flex;
  align-items: center;
  gap: 6px;
}
.control-label {
  font-size: 12px;
  color: #94a3b8;
}
.control-input {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 20px;
  padding: 6px 12px;
  color: white;
  font-size: 12px;
  width: 90px;
}
.control-input:focus {
  outline: none;
  border-color: #3b82f6;
}
.control-input:disabled {
  opacity: 0.5;
}
.control-btn {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  padding: 6px 14px;
  color: #cbd5e1;
  font-size: 12px;
  cursor: pointer;
  transition: all 0.2s;
}
.control-btn:hover:not(:disabled) {
  background: rgba(255, 255, 255, 0.15);
  transform: translateY(-1px);
}
.control-btn.start {
  background: rgba(16, 185, 129, 0.2);
  border-color: rgba(16, 185, 129, 0.4);
  color: #10b981;
}
.control-btn.pause {
  background: rgba(245, 158, 11, 0.2);
  border-color: rgba(245, 158, 11, 0.4);
  color: #fbbf24;
}
.control-btn.stop {
  background: rgba(239, 68, 68, 0.2);
  border-color: rgba(239, 68, 68, 0.4);
  color: #f87171;
}
.sim-time-display {
  font-family: monospace;
  background: rgba(0, 0, 0, 0.4);
  padding: 4px 12px;
  border-radius: 40px;
  color: #fbbf24;
  font-size: 13px;
}
.sim-status {
  display: inline-block;
  width: 10px;
  height: 10px;
  border-radius: 50%;
  margin-right: 6px;
}
.status-idle { background: #64748b; }
.status-running { background: #10b981; animation: pulse 1s infinite; }
.status-paused { background: #f59e0b; }
.status-ready { background: #3b82f6; }
@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}
.design-canvas {
  flex: 1;
  background: rgba(0, 0, 0, 0.3);
  position: relative;
  overflow: auto;
  margin: 12px;
  border-radius: 20px;
  border: 1px solid rgba(255, 255, 255, 0.08);
  min-height: 280px;
}
.design-node {
  position: absolute;
  width: 140px;
  background: rgba(20, 30, 45, 0.9);
  backdrop-filter: blur(8px);
  border: 1px solid rgba(59, 130, 246, 0.3);
  border-radius: 16px;
  padding: 10px;
  cursor: move;
  z-index: 10;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
  transition: all 0.2s;
}
.design-node:hover {
  border-color: #3b82f6;
  transform: translateY(-2px);
}
.design-node-header {
  font-weight: 600;
  font-size: 13px;
  color: white;
  display: flex;
  align-items: center;
  gap: 6px;
  margin-bottom: 6px;
}
.design-node-body {
  font-size: 11px;
  color: #94a3b8;
  margin-bottom: 8px;
}
.node-ports {
  display: flex;
  justify-content: space-between;
}
.input-port, .output-port {
  width: 14px;
  height: 14px;
  border-radius: 50%;
  background: #3b82f6;
  border: 2px solid rgba(255, 255, 255, 0.5);
}
.output-port {
  background: #f59e0b;
}
.connection {
  stroke: #3b82f6;
  stroke-width: 2;
  fill: none;
  filter: drop-shadow(0 0 2px rgba(59, 130, 246, 0.5));
}
.design-tools {
  padding: 12px 16px;
  display: flex;
  gap: 10px;
  border-top: 1px solid rgba(255, 255, 255, 0.08);
}
.tool-btn {
  background: rgba(255, 255, 255, 0.08);
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 40px;
  padding: 8px 16px;
  color: #cbd5e1;
  font-size: 13px;
  cursor: pointer;
  transition: all 0.2s;
}
.tool-btn.primary-btn {
  background: linear-gradient(135deg, #3b82f6, #8b5cf6);
  border: none;
  color: white;
}
.tool-btn:hover:not(:disabled) {
  background: rgba(255, 255, 255, 0.15);
  transform: translateY(-1px);
}
.tool-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}
.monitor-panel {
  background: rgba(15, 25, 35, 0.6);
  backdrop-filter: blur(16px);
  border: 1px solid rgba(255, 255, 255, 0.08);
  border-radius: 24px;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  height: 220px;
}
.monitor-header {
  background: linear-gradient(135deg, rgba(142, 45, 226, 0.2), rgba(74, 0, 224, 0.2));
  padding: 10px 16px;
  color: white;
  font-weight: 600;
  display: flex;
  justify-content: space-between;
  align-items: center;
  border-bottom: 1px solid rgba(255, 255, 255, 0.08);
}
.monitor-content {
  flex: 1;
  overflow-y: auto;
  padding: 12px;
  font-family: 'Consolas', monospace;
  font-size: 12px;
  background: rgba(0, 0, 0, 0.2);
  color: #cbd5e1;
}
.monitor-line {
  padding: 4px 0;
  border-bottom: 1px solid rgba(255, 255, 255, 0.05);
}
.monitor-line.info { color: #60a5fa; }
.monitor-line.error { color: #f87171; }
.monitor-line.warning { color: #fbbf24; }
.monitor-timestamp {
  color: #6a9955;
  margin-right: 12px;
}
.param-item {
  margin-bottom: 16px;
}
.param-item label {
  display: block;
  margin-bottom: 6px;
  font-size: 12px;
  color: #94a3b8;
}
.controls {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}
.flush-btn {
  background: none;
  border: none;
  color: white;
  cursor: pointer;
  font-size: 16px;
  opacity: 0.7;
  transition: opacity 0.2s;
}
.flush-btn:hover {
  opacity: 1;
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
  background: rgba(15, 25, 35, 0.95);
  backdrop-filter: blur(20px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 32px;
  width: 450px;
  max-width: 90%;
  padding: 24px;
}
.modal-content h3 {
  color: white;
  font-size: 20px;
  margin-bottom: 16px;
}
.modal-input {
  background: rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 40px;
  padding: 12px 16px;
  width: 100%;
  color: white;
  margin: 16px 0;
}
.modal-radio {
  display: flex;
  gap: 24px;
  margin: 16px 0;
  color: #cbd5e1;
}
.modal-actions {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  margin-top: 20px;
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