<template>
  <div class="center-canvas">
    <VueFlow
      :node-types="nodeTypes"
      :edge-types="edgeTypes"
      @drop="onDrop"
      @dragover="onDragOver"
      @connect="onConnect"
      @edges-delete="onEdgesDelete"
      @nodes-delete="onNodesDelete"
      class="flow-container"
      @edge-double-click="onEdgeDoubleClick"
      @node-double-click="onNodeDoubleClick"
      :delete-key-code="['Delete']"
      @node-click="({event, node}) => emit('node-selected', node)"
      :default-edge-options="defaultEdgeOptions"
      :min-zoom="0.1"
      :max-zoom="4"
      :default-viewport="{ zoom: 0.5 }"
      fit-view-on-init
    >
      <Background />
      <!-- <Controls 
        position="top-left"
        :show-interactive="false"
      >
        <ControlButton title="部署" @click="openDeployDialog">
          <Icon name="deploy" />
        </ControlButton>
        <ControlButton title="启动" @click="onStartModel">
          <Icon name="start" />
        </ControlButton>
      </Controls> -->
      <!-- <MiniMap /> -->
    </VueFlow>

    <div class="instance-name">
      <el-text size="small" class="mx-1">当前实例名:</el-text>
      <el-input
        placeholder="请输入实例名"
        v-model="instanceName"
      >
    </el-input>
    </div>

    <div class="toolbar">
      <el-button 
        size="small" 
        @click="openDeployDialog"
      >
      <template #icon>
        <Icon name="deploy" />
      </template>
        部署
      </el-button>
      <el-button 
        size="small" 
        @click="onStartModel"
      >
      <template #icon>
         <Icon name="start" />
      </template>
        启动
      </el-button>

      <el-button size="small" @click="initScene">初始化</el-button>
      <el-button size="small" @click="startScene">开始</el-button>
      <el-button size="small" @click="togglePause">{{isPause ? '恢复' : '暂停'}}</el-button>
      <el-button size="small" @click="stopScene">停止</el-button>
      <el-button size="small" @click="saveScene">保存场景</el-button>
      <!-- <el-button size="small" @click="loadScene">加载场景</el-button> -->
      <el-button size="small" type="danger" @click="clearAll">清空</el-button>
    </div>

        <!-- 部署弹窗 -->
    <el-dialog v-model="deployVisible" :title="titleName" width="700px">
      <el-table :data="deployTableData" border style="width: 100%">
        <el-table-column prop="name" label="模型名" min-width="140" />
        <el-table-column label="部署策略" min-width="160" v-if="titleName === '部署模型'">
          <template #default="{ row }">
            <el-select v-model="row.deployPolizy" placeholder="选择部署策略" clearable>
              <el-option v-for="item in strategyOptions" :key="item.value" :label="item.label" :value="item.value" />
            </el-select>
          </template>
        </el-table-column>
        <el-table-column label="部署节点" min-width="160" v-if="titleName === '部署模型'">
          <template #default="{ row }">
            <el-select v-model="row.deployIpAddr" placeholder="选择节点" clearable>
              <el-option v-for="item in nodeOptions" :key="item" :label="item" :value="item" />
            </el-select>
          </template>
        </el-table-column>

        <el-table-column label="运行配置" min-width="160" v-if="titleName === '启动模型'">
          <template #default="{ row }">
            <el-select v-model="row.executePolizy" placeholder="选择启动配置" clearable>
              <el-option v-for="item in startOptions" :key="item.value" :label="item.label" :value="item.value" />
            </el-select>
          </template>
        </el-table-column>
        <el-table-column label="步长" min-width="160" v-if="titleName === '启动模型'">
          <template #default="{ row }">
            <el-input-number 
            v-model="row.singleStep" 
            :precision="3" :step="0.001" 
            controls-position="right"
            :min="0"
            />
          </template>
        </el-table-column>

        <el-table-column label="操作" width="100" align="center" >
          <template #default="{ row }">
            <el-button type="primary" link @click="handleDeploy(row)" v-if="titleName === '部署模型'">立即部署</el-button>
            <el-button type="primary" link @click="" v-if="titleName === '启动模型'">立即启动</el-button>
          </template>
        </el-table-column>
      </el-table>
      <template #footer>
        <el-button @click="deployVisible = false">取消</el-button>
        <el-button 
          type="primary" 
          @click="handleDeployAll" 
          v-if="titleName === '部署模型'">
          全部部署
        </el-button>
        <el-button 
          type="primary" 
          @click=""
          v-if="titleName === '启动模型'"
          >
          全部启动
        </el-button>
      </template>
    </el-dialog>
  </div>
</template>

<script setup lang="ts">
import { ref,onMounted,nextTick, h } from 'vue'
import { VueFlow, useVueFlow,
    type Node,
    type Edge,
    type Connection,MarkerType,
    type DefaultEdgeOptions,
    type EdgeMouseEvent,
    type NodeMouseEvent,
 } from '@vue-flow/core'
import { Background } from '@vue-flow/background'
import { Controls,ControlButton } from '@vue-flow/controls'
import type { NodeComponent, EdgeComponent } from '@vue-flow/core'
//import { MiniMap } from '@vue-flow/minimap'
import { ElMessage, 
  ElMessageBox, 
  ElButton, 
  ElTable, ElTableColumn,
  ElDialog,ElSelect,ElOption,ElInputNumber,ElInput,ElText } from 'element-plus'
import CustomNode from './CustomNode.vue'
import CustomEdge from './CustomEdge.vue'
import type { ParsedModel, ModelInput, ModelOutput } from '@/utils/xmlParser'
import Icon from '@/components/Icon.vue'

const emit = defineEmits<{
  (e: 'node-selected', node: Node): void
}>()

const isPause = ref(false)
const instanceName = ref<string>("")
const dragModelData = ref<ParsedModel | null>(null)

const nodeTypes: Record<string, NodeComponent> = {
  'custom-node': CustomNode as NodeComponent
}
const edgeTypes: Record<string, EdgeComponent> = {
  'custom-edge': CustomEdge as EdgeComponent,
}

// 默认边配置
const defaultEdgeOptions:DefaultEdgeOptions= {
    //type: 'smoothstep',
    animated: false,
    style: { stroke: '#5a7db0', strokeWidth: 1.5 },
    markerEnd: {
        type: MarkerType.ArrowClosed,
        width: 10,
        height: 10,
        color: '#5a7db0',
    },
    label: '',
    labelStyle: { fill: '#b0b0c0', fontWeight: 500, fontSize: 11 },
    labelBgStyle: { fill: '#1e2746', rx: 6, ry: 6, opacity: 0.9 },
    labelBgPadding: [8, 5]as [number, number],
    labelBgBorderRadius: 4,
};

const { 
    addNodes,addEdges, 
    setNodes,setEdges, 
    fitView, 
    getNodes, getEdges, 
    updateNode, updateEdge,
    findNode, findEdge, 
    onNodesChange, onEdgesChange,setViewport,
    zoomTo } = useVueFlow()

// 设置拖拽数据
const setDragModelData = (data: ParsedModel) => {
  dragModelData.value = data
}

onNodesChange((changes) => {
   zoomTo(0.6)
}) 

const initScene = () => {
  ElMessage.info('初始化场景')
}

const startScene = () => {
  ElMessage.success('开始模拟')
}

const togglePause = () => {
  isPause.value = !isPause.value
  ElMessage.info(isPause.value ? '模拟已暂停' : '模拟已恢复')
}

const stopScene = () => {
  ElMessage.error('停止模拟')
}

const deployVisible = ref(false)
const deployTableData = ref<Array<{
  name: string
  deployPolizy: string
  deployIpAddr: string,
  modelUuid:string | undefined,
  executePolizy:string,
  singleStep?:number,
  aliasName?:string
}>>([])

const titleName = ref<string>("部署模型")
const strategyOptions = [
  {label: "不部署" ,value : "0"},
  {label: "仅部署一次",value : "1"},
  {label: "重新部署",value : "2"},
  {label: "版本更新时部署",value : "3"}
]

const startOptions = [
  {label: "运行" ,value : "0"},
  {label: "不运行",value : "1"}
]

const nodeOptions = ['192.168.0.1', '192.168.0.2', '192.168.0.3', '192.168.0.4']

const openDeployDialog = () => {
  // 从画布中获取所有节点作为模型列表
  const nodes = getNodes.value
  console.log(nodes)
  deployTableData.value = nodes.map(n => ({
    name: n.data.label,
    deployPolizy: "",
    deployIpAddr:"",
    executePolizy:"",
    modelUuid :n.id.split("_")[0]
  }))
  deployVisible.value = true
  titleName.value = "部署模型"
}

const handleDeploy = (row: { name: string; strategy: string; node: string }) => {
  if (!row.strategy || !row.node) {
    ElMessage.warning(`请为 "${row.name}" 选择部署策略和节点`)
    return
  }
  ElMessage.success(`正在部署 "${row.name}"，策略: ${row.strategy}，节点: ${row.node}`)
  // TODO: 调用实际部署接口
}

const handleDeployAll = () => {
  const invalidRows = deployTableData.value.filter(r => !r.deployPolizy || !r.deployIpAddr)
  if (invalidRows.length > 0) {
    ElMessage.warning(`有 ${invalidRows.length} 个模型未完整配置，请补充后再批量部署`)
    return
  }
  deployTableData.value.forEach(row => {
    handleDeploy(row)
  })
  ElMessage.success('全部模型部署请求已发送')
  deployVisible.value = false
}

const onStartModel = () => {
  ElMessage.success('启动模型')

  deployVisible.value = true
  titleName.value = "启动模型"
} 

// 拖拽放置
const onDragOver = (event: DragEvent) => {
  event.preventDefault()
  event.dataTransfer!.dropEffect = 'copy'
}

const onDrop = (event: DragEvent) => {
  event.preventDefault()
  const rawData = event.dataTransfer?.getData('application/json')
  const modelData: ParsedModel | null = rawData
    ? JSON.parse(rawData)
    : dragModelData.value

  if (!modelData) return

  const rect = (event.currentTarget as HTMLElement).getBoundingClientRect()
  const position = {
    x: event.clientX - rect.left - 150,
    y: event.clientY - rect.top - 100,
  }
  addNodeFromModel(modelData, position)
}

// 从模型数据创建节点
const addNodeFromModel = (model: ParsedModel, position: { x: number; y: number }) => {
  const newNodeId = `${model.uuid}_${Date.now()}`
  const newNode: Node = {
    id: newNodeId,
    type: 'custom-node',
    position,
    data: {
      model: model,
      inputs: model.inputs,
      outputs: model.outputs,
      label: model.displayName || model.name,
    },
  }
 
  addNodes([newNode])

  ElMessage.success(`已添加节点: ${model.displayName || model.name}`)
}

// 连线验证并创建
const onConnect = (connection: Connection) => {
  const sourceNode = findNode(connection.source)
  const targetNode = findNode(connection.target)
  if (!sourceNode || !targetNode) return

  const sourceOutput = sourceNode.data.outputs.find((o: ModelOutput) => `output-${o.name}` === connection.sourceHandle)
  const targetInput = targetNode.data.inputs.find((i: ModelInput) => `input-${i.name}` === connection.targetHandle)

  if (!sourceOutput || !targetInput) {
    ElMessage.warning('无效的Handle')
    return
  }

  // 类型验证：比较type uuid
  if (sourceOutput.type !== targetInput.type) {
    ElMessage.error(`类型不匹配: ${sourceOutput.type} vs ${targetInput.type}`)
    return
  }

  const edgeId = `edge-${connection.source}-${connection.sourceHandle}-${connection.target}-${connection.targetHandle}`
  const newEdge: Edge = {
    id: edgeId,
    source: connection.source!,
    target: connection.target!,
    sourceHandle: connection.sourceHandle,
    targetHandle: connection.targetHandle,
    type: 'custom-edge',
    label: sourceOutput.alias || sourceOutput.name,
    data: {
      alias: sourceOutput.alias || sourceOutput.name,
      originalAlias: sourceOutput.alias || sourceOutput.name,
    },
  }
  addEdges([newEdge])
}

// 删除连线
const onEdgesDelete = (edges: Edge[]) => {
  ElMessage.success(`已删除 ${edges.length} 条连线`)
}

// 删除节点
const onNodesDelete = (nodes: Node[]) => {
  ElMessage.success(`已删除 ${nodes.length} 个节点`)
}

// 双击节点修改名称
const onNodeDoubleClick = ({node}:NodeMouseEvent) => {
  ElMessageBox.prompt('请输入新的节点名称', '编辑节点', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    inputValue: node.data.label,
  }).then(({ value }) => {
    if (value) {
      node.data.label = value
      updateNode(node.id, { data: { ...node.data, label: value } })
      ElMessage.success('节点名称已更新')
    }
  })
}

// 双击边修改alias
const onEdgeDoubleClick = ({ edge }: EdgeMouseEvent) => {
  ElMessageBox.prompt('请输入新的连线名称 (alias)', '编辑连线', {
    confirmButtonText: '确定',
    cancelButtonText: '取消',
    inputValue: edge.data?.alias || edge.label,
  }).then(({ value }) => {
    if (value) {
      setEdges((edges) =>
        edges.map((e) =>
          e.id === edge.id
            ? {
                ...e,
                label: value,
                data: {
                  ...e.data,
                  alias: value,
                  originalAlias: e.data?.originalAlias || value,
                },
              }
            : e
        )
      )
      ElMessage.success('连线名称已更新')
    }
  })
}

// 保存场景到localStorage
const saveScene = () => {
  const nodes = getNodes.value.map(n => ({
    id: n.id,
    position: n.position,
    data: {
      model: n.data.model,
      label: n.data.label,
    }
  }))
  const edges = getEdges.value.map(e => ({
    id: e.id,
    source: e.source,
    target: e.target,
    sourceHandle: e.sourceHandle,
    targetHandle: e.targetHandle,
    label: e.label,
    data: e.data,
  }))
  console.log('Saving scene:', { nodes, edges })

  //根据节点和边得配置生成设计Xml
  const designXml = generateDesignXml(nodes, edges)
  //todo 生成type xml
  const typeXml = generateTypeXml(nodes)
  //todo 生成初始化参数配置xml
  console.log("design",designXml)
  console.log("typexml",typeXml)

  const sceneData = { nodes, edges, version: '1.0' }
  localStorage.setItem('vueflow_scene', JSON.stringify(sceneData))
  ElMessage.success('场景已保存')
}

//生成设计xml
const generateDesignXml = (nodes:any, edges: any) :string=> {
  //todo 根据节点和边得配置生成设计Xml
  const xmlDoc = document.implementation.createDocument(null, 'root', null);
  const root = xmlDoc.documentElement;

  for (const node of nodes) {
    const model = xmlDoc.createElement('model');
    const modelUuid = node.id.split('_')[0]
    model.setAttribute('type', modelUuid); 
    model.setAttribute('instance', node.data.label);
    model.setAttribute('address', "");
    model.setAttribute('pid', "1");

    const timers = xmlDoc.createElement('timers');
    const timer = xmlDoc.createElement('timer');
    timer.setAttribute('name', '');
    timer.setAttribute('sync_type', 'SYNC_GLOBAL');
    timer.setAttribute('interval', '1000');
    timers.appendChild(timer);

    const inputs = xmlDoc.createElement('inputs');
    for (const edge of edges) {
      if (edge.target.split('_')[0] === modelUuid) {
        const input = xmlDoc.createElement('input');
        input.setAttribute('name', edge.targetHandle.replace('input-', ''));
        input.setAttribute('topic', edge.label); 

        createIOChildEle(input,xmlDoc)
        inputs.appendChild(input);
      }
    }
  
    const outputs = xmlDoc.createElement('outputs');

    for (const edge of edges) {
      if (edge.source.split('_')[0] === modelUuid) {
        const output = xmlDoc.createElement('output');
        output.setAttribute('name', edge.sourceHandle.replace('output-', ''));
        output.setAttribute('topic', edge.data?.originalAlias || edge.label); 

        createIOChildEle(output,xmlDoc)
        outputs.appendChild(output);
      }
    }

    model.appendChild(timers);
    model.appendChild(inputs);
    model.appendChild(outputs);
    root.appendChild(model);
  }

  const serializer = new XMLSerializer();
  return '<?xml version="1.0" encoding="utf-8" standalone="yes"?>' + serializer.serializeToString(xmlDoc)
}

//生成数据类型xml
const generateTypeXml = (nodes:any):string=>{
  //todo 根据节点和边得配置生成设计Xml
  const xmlDoc = document.implementation.createDocument(null, 'root', null);
  const root = xmlDoc.documentElement;

  const set = new Set<string>()
  const parser = new DOMParser()
  const typesEle = xmlDoc.createElement('types')
  for (const node of nodes) {
    const model:ParsedModel = node.data.model
    const types = model.types || []
    for(const type of types){ 
      if(set.has(type.uuid)) continue
      set.add(type.uuid)
      const typeEle = parser.parseFromString(type.typeElement || '', 'text/xml')
      typesEle.appendChild(typeEle.documentElement);
    }
  }
  root.appendChild(typesEle);

  const serializer = new XMLSerializer();
  return '<?xml version="1.0" encoding="utf-8" standalone="yes"?>' + serializer.serializeToString(xmlDoc)
}

const createIOChildEle = (iOEle: Element, xmlDoc: Document) => {
  const reliable = xmlDoc.createElement('reliable');
  reliable.textContent = 'true';
  const minimum_trans_interval = xmlDoc.createElement('minimum_trans_interval');
  minimum_trans_interval.setAttribute('status', 'disable');
  minimum_trans_interval.textContent = '0';
  const timeout_unsent_warning = xmlDoc.createElement('timeout_unsent_warning');
  timeout_unsent_warning.textContent = '140704677232953';
  const multicast = xmlDoc.createElement('multicast');
  multicast.setAttribute('address', '');
  multicast.setAttribute('port', '0');

  iOEle.appendChild(reliable);
  iOEle.appendChild(minimum_trans_interval);
  iOEle.appendChild(timeout_unsent_warning);
  iOEle.appendChild(multicast);
}

// 加载场景
const loadScene = async () => {
  const raw = localStorage.getItem('vueflow_scene')
  if (!raw) {
    ElMessage.warning('无保存的场景')
    return
  }
  try {
    const scene = JSON.parse(raw)
    const restoredNodes = scene.nodes.map((n: any) => ({
      ...n,
      type: 'custom-node',
      data: {
        model: n.data.model,
        inputs: n.data.model.inputs,
        outputs: n.data.model.outputs,
        label: n.data.label,
      }
    }))
    // 使用 useVueFlow 的 API 将节点/连线写入内部状态，避免 elements 和内部状态不同步
    setNodes(restoredNodes)
    setEdges(scene.edges)

    // await nextTick(async ()=>{
    //   await fitView({ 
    //       padding: 0.12, 
    //       includeHiddenNodes: true, 
    //       minZoom: 0.1, 
    //       maxZoom: 4, 
    //   })
    // })
    //await zoomTo(0.5)
    ElMessage.success('场景加载成功，已自动调整视图')
  } catch (e) {
    ElMessage.error('场景加载失败')
  }
}

const clearAll = () => {
  setNodes([])
  setEdges([])
  ElMessage.info('已清空画布')
}

onMounted(()=>{
  loadScene()
})


defineExpose({ setDragModelData })
</script>

<style scoped>
.center-canvas {
  width: 100%;
  height: 100%;
  position: relative;
  padding: 12px;
  box-sizing: border-box;
  background: #ffffff;
  border-radius: 12px;
  border: 1px solid rgba(0, 0, 0, 0.06);
  box-shadow: 0 1px 8px rgba(0, 0, 0, 0.06);
  overflow: hidden;
}
.flow-container {
  width: 100%;
  height: 100%;
  border-radius: 10px;
}
.toolbar {
  position: absolute;
  top: 16px;
  right: 16px;
  display: flex;
  gap: 2px;
  z-index: 10;
}

.instance-name {
  height: 24px;
  display: flex;
  align-items: center;
  position: absolute;
  top: 16px;
  left: 16px;
  gap: 2px;
  z-index: 10;
}
.mx-1{
  min-width: 70px;
  font-size: 12px;
  height: 24px;
  display: flex;
  align-items: center;
}

.instance-name .el-input{
  height: 24px;
}


/* .vue-flow__controls {
    display:flex;
    flex-wrap:wrap;
    justify-content:center
} */
</style>