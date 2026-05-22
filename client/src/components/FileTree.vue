// components/FileTree.vue
<template>
  <div class="file-tree-parent">
    <h4>模型列表</h4>
    <el-tree
      class="file-tree-child"
      :data="treeData"
      :props="defaultProps"
      node-key="id"
    >
      <template #default="{ node, data }">
        <span v-if="data.type === 'file'" class="file-node">
          <el-icon color="#ffd251"><FolderOpened /></el-icon>
          <span>{{ data.label }}</span>
        </span>
        <span v-else class="model-node" draggable="true" @dragstart="onModelDragStart($event, data)">
          <el-icon ><Document /></el-icon>
          <span>{{ data.label }}</span>
        </span>
      </template>
    </el-tree>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { ElMessage,ElButton,ElTree,ElIcon  } from 'element-plus'
import { Document,FolderOpened } from '@element-plus/icons-vue'
import { parseXMLToModels, type ParsedModel } from '@/utils/xmlParser'


interface TreeNode {
  id: string
  label: string
  type: 'file' | 'model'
  children?: TreeNode[]
  modelData?: ParsedModel
}

const emit = defineEmits<{
  (e: 'node-drag-start', modelData: ParsedModel): void
}>()

const treeData = ref<TreeNode[]>([])
const defaultProps = {
  children: 'children',
  label: 'label',
}

// 解析XML并构建树
const loadXMLFile = async (fileName: string, xmlContent: string) => {
  const models = parseXMLToModels(xmlContent)
  const modelChildren: TreeNode[] = models.map(model => ({
    id: `${fileName}_${model.name}`,
    label: model.displayName || model.name,
    type: 'model',
    modelData: model,
  }))
  return {
    id: fileName,
    label: fileName,
    type: 'file',
    children: modelChildren,
  }
}

// 初始化内置示例文件
const initSampleFiles = async () => {
  const files = [
    { name: 'EmbodiedRobot.xml', content: `<?xml version="1.0" encoding="UTF-8"?>
<root type="SERVICE">
 <types>
  <type>
   <info alias="RobotConfig" create_time="" creator="" last_modify="" modifier="" name="RobotConfig" note="机器人动力学模型相关的配置参数" type="TYPE_STRUCT" uuid="06d9b299-8992-4e9d-a6c2-105421dd5d91" version=""/>
   <member array="" display="real32" display_name="sim_step" elem_type="--" name="sim_step" note="仿真步长" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="int16" display_name="model_type" elem_type="--" name="model_type" note="模型的类型（目前只有四轮）" range="" sequence_type="" type="int16" unit_name=""/>
   <member array="" display="Vector3" display_name="init_location" elem_type="--" name="init_location" note="初始位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="init_rotation" elem_type="--" name="init_rotation" note="初始姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="real32" display_name="max_speed" elem_type="--" name="max_speed" note="最大速度" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="max_yawrate" elem_type="--" name="max_yawrate" note="最大角速率" range="" sequence_type="" type="real32" unit_name=""/>
  </type>
  <type>
   <info alias="Vector3" create_time="" creator="" last_modify="" modifier="" name="Vector" note="三维向量" type="TYPE_STRUCT" uuid="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" version=""/>
   <member array="" display="real32" display_name="x(x)" elem_type="--" name="x" note="x轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="y(y)" elem_type="--" name="y" note="y轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="z(z)" elem_type="--" name="z" note="z轴" range="" sequence_type="" type="real32" unit_name="m"/>
  </type>
  <type>
   <info alias="ControlSignal" create_time="" creator="" last_modify="" modifier="" name="ControlSignal" note="控制输出指令" type="TYPE_STRUCT" uuid="9232b8bd-8115-4ffa-8161-d55f307caa03" version=""/>
   <member array="" display="real32" display_name="yaw_rate" elem_type="--" name="yaw_rate" note="转向角速率" range="" sequence_type="" type="real32" unit_name="rad/s"/>
   <member array="" display="real32" display_name="velocity" elem_type="--" name="velocity" note="线速度" range="" sequence_type="" type="real32" unit_name="m/s"/>
  </type>
  <type>
   <info alias="RoadData" create_time="" creator="" last_modify="" modifier="" name="RoadData" note="高程信息" type="TYPE_STRUCT" uuid="61cf8616-78ee-4d0e-b902-ad1bce69c76f" version=""/>
   <member array="" display="sequence" display_name="value" elem_type="Array4" name="value" note="" range="" sequence_type="46838e04-c6a0-4c8d-8c84-bb048a50eb12" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Array4" create_time="" creator="" last_modify="" modifier="" name="Array4" note="Array4" type="TYPE_STRUCT" uuid="46838e04-c6a0-4c8d-8c84-bb048a50eb12" version=""/>
   <member array="" display="real64" display_name="x" elem_type="--" name="x" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="y" elem_type="--" name="y" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="z" elem_type="--" name="z" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="mue" elem_type="--" name="mue" note="" range="" sequence_type="" type="real64" unit_name=""/>
  </type>
  <type>
   <info alias="ImuData" create_time="" creator="" last_modify="" modifier="" name="IMUDATA" note="IMU的数据结构体信息" type="TYPE_STRUCT" uuid="9b4de777-49d4-44a0-9ba4-d480d9589b01" version=""/>
   <member array="" display="Header" display_name="header" elem_type="" name="header" note="数据header描述" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="Vector4" display_name="orientation" elem_type="" name="orientation" note="姿态四元数" range="" sequence_type="" type="074fc1e2-3d50-4e12-b3ea-15af58b2ecb4" unit_name=""/>
   <member array="" display="sequence" display_name="orientation_covariance" elem_type="real32" name="orientation_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
   <member array="" display="Vector" display_name="angular_velocity" elem_type="" name="angular_velocity" note="角速率值" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="rad/s"/>
   <member array="" display="sequence" display_name="angular_velocity_covariance" elem_type="real32" name="angular_velocity_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
   <member array="" display="Vector" display_name="linear_acceleration" elem_type="" name="linear_acceleration" note="加速度值" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="m/s2"/>
   <member array="" display="sequence" display_name="linear_acceleration_covariance" elem_type="real32" name="linear_acceleration_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Header" create_time="" creator="" last_modify="" modifier="" name="HEADER" note="header描述文件" type="TYPE_STRUCT" uuid="17f6811e-067e-4a7d-8ef4-ac528fe1f047" version=""/>
   <member array="" display="string" display_name="frame_id" elem_type="--" name="frame_id" note="数据所处的坐标系" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="Stamp" display_name="stamp" elem_type="" name="stamp" note="时间戳相关的信息" range="" sequence_type="" type="7fcc7a13-83bd-4fd4-91d8-026f7631c086" unit_name=""/>
  </type>
  <type>
   <info alias="Stamp" create_time="" creator="" last_modify="" modifier="" name="Stamp" note="主要用于记录时间戳相关的信息" type="TYPE_STRUCT" uuid="7fcc7a13-83bd-4fd4-91d8-026f7631c086" version=""/>
   <member array="" display="int32" display_name="sec" elem_type="--" name="sec" note="时间戳，表示秒的部分" range="" sequence_type="" type="int32" unit_name="秒"/>
   <member array="" display="uint32" display_name="nanosec" elem_type="--" name="nanosec" note="时间戳，表示纳秒的部分" range="" sequence_type="" type="uint32" unit_name="纳秒"/>
  </type>
  <type>
   <info alias="Vector4" create_time="" creator="" last_modify="" modifier="" name="Vector4" note="四维向量" type="TYPE_STRUCT" uuid="074fc1e2-3d50-4e12-b3ea-15af58b2ecb4" version=""/>
   <member array="" display="real32" display_name="字段1" elem_type="--" name="x" note="四维数x" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段2" elem_type="--" name="y" note="四维数y" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段3" elem_type="--" name="z" note="四维数z" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段4" elem_type="--" name="w" note="四维数w" range="" sequence_type="" type="real32" unit_name=""/>
  </type>
  <type>
   <info alias="RobotInfo" create_time="" creator="" last_modify="" modifier="" name="RobotInfo" note="UE中用于机器人的更新信息" type="TYPE_STRUCT" uuid="6fe0e80e-e781-4ff1-934f-de2cf07f2cc7" version=""/>
   <member array="" display="real32" display_name="sim_time" elem_type="--" name="sim_time" note="仿真时间" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="Vector3" display_name="location" elem_type="--" name="location" note="机器人运动学和动力学模型的位置信息" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="m"/>
   <member array="" display="Vector3" display_name="rotation" elem_type="--" name="rotation" note="机器人的姿态信息（滚转，俯仰，偏航的顺序）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="°"/>
  </type>
 </types>
 <models>
  <model>
   <info address="" display_name="EmbodiedRobot" language="c++" name="EmbodiedRobot" note="运行多种具身智能实体，包括四足机器人，轮式机器人，轮履机器人等运动学和动力学模拟; 主要通过mujoco集中到ros2里面" person="巫永舟" software="mujoco" type="TYPE_MODEL" uuid="52e0edf7-ec44-4e6a-b8c9-ecae678ba058" version=""/>
   <members>
    <member display="display" name="robotconfig" note="模型的初始化配置参数" type="06d9b299-8992-4e9d-a6c2-105421dd5d91"/>
   </members>
   <inputs>
    <input alias="ControlSignal_topic" display="display" name="control_cmd" note="控制指令，接收相应的信号" type="9232b8bd-8115-4ffa-8161-d55f307caa03"/>
    <input alias="RoadData_topic" display="display" name="road_data" note="以机器人中心为原点，一定范围内的高度图" type="61cf8616-78ee-4d0e-b902-ad1bce69c76f"/>
   </inputs>
   <outputs>
    <output alias="IMUDATA_topic" display="display" name="imu" note="IMU传感器位置" type="9b4de777-49d4-44a0-9ba4-d480d9589b01"/>
    <output alias="RobotInfo_topic" display="display" name="robot_info" note="description" type="6fe0e80e-e781-4ff1-934f-de2cf07f2cc7"/>
   </outputs>
   <threads>
    <thread interval="" name="run_0" note="description" sync_type="" timer_type=""/>
   </threads>
  </model>
 </models>
 <service>
  <info name="EmbodiedRobot" type="TYPE_SERVICE" uuid="EmbodiedRobot"/>
  <members/>
  <models>
   <model model="52e0edf7-ec44-4e6a-b8c9-ecae678ba058"/>
  </models>
 </service>
</root>
` },
    { name: 'VirtualScence.xml', content: `<?xml version="1.0" encoding="UTF-8"?>
<root type="SERVICE">
 <types>
  <type>
   <info alias="UnrealEnvConfig（虚拟场景参数配置）" create_time="" creator="" last_modify="" modifier="" name="UnrealEnvConfig" note="Unreal中的环境信息" type="TYPE_STRUCT" uuid="6d71b57e-3bba-4ae2-9e34-b7b47fe664ad" version=""/>
   <member array="" display="real32" display_name="sim_step（仿真步长）" elem_type="--" name="sim_step" note="仿真步长" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="UnrealNaturalEnvConfig" display_name="natural_env（环境配置）" elem_type="--" name="natural_env_config" note="自然环境配置，包括相关的地图信息" range="" sequence_type="" type="01fcc6c3-2566-4130-81c3-96f418303ce2" unit_name=""/>
   <member array="" display="sequence" display_name="static_env（静态实体）【下拉菜单】" elem_type="UnrealStaticEntity" name="static_env_config" note="静态环境实体，如各种静态障碍物等；爆炸物，引爆点也归为此类" range="" sequence_type="c4398684-b477-4e19-8ee5-7f1a2e2fdc47" type="sequence" unit_name=""/>
   <member array="" display="sequence" display_name="dynamic_env（动态实体）【下拉菜单】" elem_type="UnrealDynamicEntity" name="dynamic_env_config" note="动态环境实体" range="" sequence_type="6a3bd2b2-bca3-4a36-a9a6-34759e61ce5c" type="sequence" unit_name=""/>
   <member array="" display="sequence" display_name="robots（机器人）【下拉菜单】" elem_type="UnrealRobotEntity" name="robots_config" note="机器人信息" range="" sequence_type="fdea77e5-e359-4860-92db-eec161ec601c" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealNaturalEnvConfig" create_time="" creator="" last_modify="" modifier="" name="UnrealNaturalEnvConfig" note="自然环境的配置信息" type="TYPE_STRUCT" uuid="01fcc6c3-2566-4130-81c3-96f418303ce2" version=""/>
   <member array="" display="string" display_name="map（场景）【下拉菜单】" elem_type="--" name="map" note="unreal中的map,目前主要有两个选择的场景，(沙漠边境：&quot;desert&quot;，高山峡谷：“mountain”)" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="int8" display_name="road_type(道路类型)【下拉菜单】" elem_type="--" name="road_type" note="unreal中的道路类型，0，沥青； 1，泥泞； 2，水洼" range="" sequence_type="" type="int8" unit_name=""/>
   <member array="" display="Vector3" display_name="ori_positon（初始位置）" elem_type="--" name="ori_positon" note="map选定的（0,0,0）坐标，对应的unreal中map中的位置坐标，核心为其转换关系" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="ori_rotation（初始姿态）" elem_type="--" name="ori_rotation" note="map选定的（0,0,0）坐标，对应的unreal中map中的坐标的角度偏差" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="real32" display_name="光照亮度（-）" elem_type="--" name="Illuminance" note="光照亮度" range="" sequence_type="" type="real32" unit_name="cd/㎡"/>
   <member array="" display="real32" display_name="降雨量（-）" elem_type="--" name="rainfall" note="降雨量" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="降雪量（-）" elem_type="--" name="snowfall" note="降雪量" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="昼夜（昼夜）【下拉菜单】" elem_type="--" name="daynight" note="设置时间（白天或者黑夜）" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="rain_snow（-）" elem_type="--" name="rain_snow" note="雨加雪（0是雨，1是雪）" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="int32" display_name="fog_level(雾等级)【下拉菜单】" elem_type="--" name="fog_level" note="雾等级" range="" sequence_type="" type="int32" unit_name=""/>
  </type>
  <type>
   <info alias="Vector3" create_time="" creator="" last_modify="" modifier="" name="Vector" note="三维向量" type="TYPE_STRUCT" uuid="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" version=""/>
   <member array="" display="real32" display_name="x(x)" elem_type="--" name="x" note="x轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="y(y)" elem_type="--" name="y" note="y轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="z(z)" elem_type="--" name="z" note="z轴" range="" sequence_type="" type="real32" unit_name="m"/>
  </type>
  <type>
   <info alias="UnrealStaticEntity" create_time="" creator="" last_modify="" modifier="" name="UnrealStaticEntity" note="静态环境要素配置" type="TYPE_STRUCT" uuid="c4398684-b477-4e19-8ee5-7f1a2e2fdc47" version=""/>
   <member array="" display="string" display_name="type（模型类型）" elem_type="--" name="type" note="实体类型，如，弹药" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="string" display_name="model_name（-）" elem_type="--" name="model_name" note="模型名称" range="" sequence_type="" type="string" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealDynamicEntity" create_time="" creator="" last_modify="" modifier="" name="UnrealDynamicEntity" note="动态环境要素配置" type="TYPE_STRUCT" uuid="6a3bd2b2-bca3-4a36-a9a6-34759e61ce5c" version=""/>
   <member array="" display="string" display_name="type（模型类型）" elem_type="--" name="type" note="实体类型，如，车辆" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="string" display_name="model_name（-）" elem_type="--" name="model_name" note="模型名称" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="sequence" display_name="waypoints(运动路径)" elem_type="Vector3" name="waypoints" note="运动路径" range="" sequence_type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" type="sequence" unit_name=""/>
   <member array="" display="real32" display_name="speed(移动速度)" elem_type="--" name="speed" note="移动速度" range="" sequence_type="" type="real32" unit_name="m/s"/>
  </type>
  <type>
   <info alias="UnrealRobotEntity" create_time="" creator="" last_modify="" modifier="" name="UnrealRobotEntity" note="Unreal的机器人配置" type="TYPE_STRUCT" uuid="fdea77e5-e359-4860-92db-eec161ec601c" version=""/>
   <member array="" display="UnrealSensorConfig" display_name="robot_sensor（机器人传感器）" elem_type="--" name="robot_sensor" note="" range="" sequence_type="" type="2d37530f-33db-4cb4-a84b-5f38317816db" unit_name=""/>
   <member array="" display="int32" display_name="robot_type（机器人类型）【下拉菜单】" elem_type="--" name="robot_type" note="机器人类型（不同类型的机器人设置）0：轮履车； 1：四轮机器人； 2：四足机器人； 3：人形机器人； 4：无人机" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="int32" display_name="release（-）" elem_type="--" name="release" note="" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="boom（-）" elem_type="--" name="boom" note="" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="RobotJoint" display_name="robot_joint（机械臂设置）" elem_type="--" name="robot_joint" note="机械臂设置" range="" sequence_type="" type="37f228b6-1ffa-44cd-81f6-20dd6f4dafda" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealSensorConfig" create_time="" creator="" last_modify="" modifier="" name="UnrealSensorConfig" note="unreal传感器的参数配置" type="TYPE_STRUCT" uuid="2d37530f-33db-4cb4-a84b-5f38317816db" version=""/>
   <member array="" display="sequence" display_name="lidar_sensors（激光传感器）" elem_type="UnrealLidarConfig" name="lidar_sensors" note="激光传感器配置" range="" sequence_type="17b88a59-732c-4ca3-b454-b4e4a2c59126" type="sequence" unit_name=""/>
   <member array="" display="sequence" display_name="image_sensors（图像传感器）" elem_type="UnrealImageConfig" name="image_sensors" note="图像传感器配置" range="" sequence_type="c25404a8-ff99-4c3e-b9e9-ff9b26cd521b" type="sequence" unit_name=""/>
   <member array="" display="sequence" display_name="road_sensors（地形传感器）" elem_type="UnrealTofConfig" name="road_sensors" note="地形传感器" range="" sequence_type="d9f56c15-0045-47a9-bfd8-5c507cf17d7d" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealLidarConfig" create_time="" creator="" last_modify="" modifier="" name="UnrealLidarConfig" note="激光传感器参数配置" type="TYPE_STRUCT" uuid="17b88a59-732c-4ca3-b454-b4e4a2c59126" version=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="int32" display_name="channel（线束）" elem_type="--" name="channel" note="" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="real32" display_name="resolution（分辨率）" elem_type="--" name="resolution" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="distance（探测距离）" elem_type="--" name="distance" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="hor_start（-）" elem_type="--" name="hor_start" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="hor_end（-）" elem_type="--" name="hor_end" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="ver_start（-）" elem_type="--" name="ver_start" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="ver_end（-）" elem_type="--" name="ver_end" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="scan_freq（频率）" elem_type="--" name="scan_freq" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="sim_step（仿真步长）" elem_type="--" name="sim_step" note="" range="" sequence_type="" type="real32" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealImageConfig" create_time="" creator="" last_modify="" modifier="" name="UnrealImageConfig" note="图像传感器配置参数（暂时只考虑简单参数配置）" type="TYPE_STRUCT" uuid="c25404a8-ff99-4c3e-b9e9-ff9b26cd521b" version=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="相对轮履机器人的坐标（右手坐标系）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="相对轮履机器人的坐标（右手坐标系）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="int32" display_name="width（宽度）" elem_type="--" name="width" note="图像的宽度" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="height（高度）" elem_type="--" name="height" note="图像的高度" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="hz（频率）" elem_type="--" name="hz" note="图像发送的频率" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="fov（视角）" elem_type="--" name="fov" note="相机的视场角" range="" sequence_type="" type="int32" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealTofConfig" create_time="" creator="" last_modify="" modifier="" name="UnrealTofConfig" note="图像Tof传感器配置参数" type="TYPE_STRUCT" uuid="d9f56c15-0045-47a9-bfd8-5c507cf17d7d" version=""/>
   <member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="相对轮履机器人的坐标（右手坐标系）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="相对轮履机器人的坐标（右手坐标系）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="int32" display_name="width（宽度）" elem_type="--" name="width" note="图像的宽度" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="height（高度）" elem_type="--" name="height" note="图像的高度" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="hz（频率）" elem_type="--" name="hz" note="图像发送的频率" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="fov（频率）" elem_type="--" name="fov" note="相机的视场角" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="body_show（-）" elem_type="--" name="body_show" note="" range="" sequence_type="" type="int32" unit_name=""/>
  </type>
  <type>
   <info alias="UnrealRobotJoint" create_time="" creator="" last_modify="" modifier="" name="UnrealRobotJoint" note="机械臂" type="TYPE_STRUCT" uuid="37f228b6-1ffa-44cd-81f6-20dd6f4dafda" version=""/>
   <member array="4" display="real32" display_name="Front Left（Front Left）" elem_type="--" name="front_left" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="4" display="real32" display_name="Front Right（Front Right）" elem_type="--" name="front_right" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="4" display="real32" display_name="Back Left（Back Left）" elem_type="--" name="back_left" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="4" display="real32" display_name="Back Right（Back Right）" elem_type="--" name="back_right" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="11" display="real32" display_name="Arm Left（Arm Left）" elem_type="--" name="arm_left" note="" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="11" display="real32" display_name="Arm Right（Arm Right）" elem_type="--" name="arm_right" note="" range="" sequence_type="" type="real32" unit_name=""/>
  </type>
  <type>
   <info alias="Bytes" create_time="" creator="" last_modify="" modifier="" name="Bytes" note="字符串流" type="TYPE_STRUCT" uuid="c9f3d3ef-a302-4ef5-9f24-4135c3cdd9c3" version=""/>
   <member array="" display="sequence" display_name="value" elem_type="int8" name="value" note="" range="" sequence_type="int8" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="RobotInfo" create_time="" creator="" last_modify="" modifier="" name="RobotInfo" note="UE中用于机器人的更新信息" type="TYPE_STRUCT" uuid="6fe0e80e-e781-4ff1-934f-de2cf07f2cc7" version=""/>
   <member array="" display="real32" display_name="sim_time" elem_type="--" name="sim_time" note="仿真时间" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="Vector3" display_name="location" elem_type="--" name="location" note="机器人运动学和动力学模型的位置信息" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="m"/>
   <member array="" display="Vector3" display_name="rotation" elem_type="--" name="rotation" note="机器人的姿态信息（滚转，俯仰，偏航的顺序）" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="°"/>
  </type>
  <type>
   <info alias="ImageData" create_time="" creator="" last_modify="" modifier="" name="ImageData" note="图像数据输入" type="TYPE_STRUCT" uuid="ae19c0f4-f801-404c-952f-aaff50572861" version=""/>
   <member array="" display="Header" display_name="字段1" elem_type="--" name="header" note="" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="uint32" display_name="字段2" elem_type="--" name="height" note="" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="uint32" display_name="字段3" elem_type="--" name="width" note="" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="uint8" display_name="字段4" elem_type="--" name="is_bigendian" note="" range="" sequence_type="" type="uint8" unit_name=""/>
   <member array="" display="uint32" display_name="字段5" elem_type="--" name="step" note="每一行像素数据的字节数" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="string" display_name="字段7" elem_type="--" name="encoding" note="指定图像数据的编码格式" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="sequence" display_name="字段6" elem_type="uint8" name="data" note="图像数据存放位置" range="" sequence_type="uint8" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Header" create_time="" creator="" last_modify="" modifier="" name="HEADER" note="header描述文件" type="TYPE_STRUCT" uuid="17f6811e-067e-4a7d-8ef4-ac528fe1f047" version=""/>
   <member array="" display="string" display_name="frame_id" elem_type="--" name="frame_id" note="数据所处的坐标系" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="Stamp" display_name="stamp" elem_type="" name="stamp" note="时间戳相关的信息" range="" sequence_type="" type="7fcc7a13-83bd-4fd4-91d8-026f7631c086" unit_name=""/>
  </type>
  <type>
   <info alias="Stamp" create_time="" creator="" last_modify="" modifier="" name="Stamp" note="主要用于记录时间戳相关的信息" type="TYPE_STRUCT" uuid="7fcc7a13-83bd-4fd4-91d8-026f7631c086" version=""/>
   <member array="" display="int32" display_name="sec" elem_type="--" name="sec" note="时间戳，表示秒的部分" range="" sequence_type="" type="int32" unit_name="秒"/>
   <member array="" display="uint32" display_name="nanosec" elem_type="--" name="nanosec" note="时间戳，表示纳秒的部分" range="" sequence_type="" type="uint32" unit_name="纳秒"/>
  </type>
  <type>
   <info alias="LidarData" create_time="" creator="" last_modify="" modifier="" name="LidarData" note="激光传感器数据" type="TYPE_STRUCT" uuid="36f998e7-1ac3-4e24-95d1-1eb8a291071d" version=""/>
   <member array="" display="Header" display_name="字段1" elem_type="" name="header" note="激光传感器的帧头数据" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="sequence" display_name="字段2" elem_type="LidarPoint" name="points" note="激光传感器中的点相关信息" range="" sequence_type="a45f9442-1645-42b8-b26d-e63e975e5fe3" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="LidarPoint" create_time="" creator="" last_modify="" modifier="" name="LidarPoint" note="激光点数据结构体" type="TYPE_STRUCT" uuid="a45f9442-1645-42b8-b26d-e63e975e5fe3" version=""/>
   <member array="" display="real32" display_name="字段1" elem_type="--" name="x" note="位置x" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段2" elem_type="--" name="y" note="位置y" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段3" elem_type="--" name="z" note="位置z" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="uint8" display_name="字段4" elem_type="--" name="intensity" note="反射率" range="" sequence_type="" type="uint8" unit_name=""/>
   <member array="" display="uint16" display_name="字段5" elem_type="--" name="ring" note="通道" range="" sequence_type="" type="uint16" unit_name=""/>
   <member array="" display="real64" display_name="字段6" elem_type="--" name="time" note="时间" range="" sequence_type="" type="real64" unit_name=""/>
  </type>
  <type>
   <info alias="RoadData" create_time="" creator="" last_modify="" modifier="" name="RoadData" note="高程信息" type="TYPE_STRUCT" uuid="61cf8616-78ee-4d0e-b902-ad1bce69c76f" version=""/>
   <member array="" display="sequence" display_name="value" elem_type="Array4" name="value" note="" range="" sequence_type="46838e04-c6a0-4c8d-8c84-bb048a50eb12" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Array4" create_time="" creator="" last_modify="" modifier="" name="Array4" note="Array4" type="TYPE_STRUCT" uuid="46838e04-c6a0-4c8d-8c84-bb048a50eb12" version=""/>
   <member array="" display="real64" display_name="x" elem_type="--" name="x" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="y" elem_type="--" name="y" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="z" elem_type="--" name="z" note="" range="" sequence_type="" type="real64" unit_name=""/>
   <member array="" display="real64" display_name="mue" elem_type="--" name="mue" note="" range="" sequence_type="" type="real64" unit_name=""/>
  </type>
 </types>
 <models>
  <model>
   <info address="" display_name="VirtualScence" language="C++，蓝图" name="VirtualScence" note="" person="" software="UE5" type="TYPE_MODEL" uuid="c1d4d6b4-d0ac-438d-8ba9-bc1a686a62ad" version=""/>
   <members>
    <member display="场景初始化需要的配置参数" name="Ueconfig" note="场景初始化需要的配置参数" type="6d71b57e-3bba-4ae2-9e34-b7b47fe664ad"/>
   </members>
   <inputs>
    <input alias="Bytes_topic" display="机器人位置姿态" name="RobotInfo" note="字符串流形式输入" type="c9f3d3ef-a302-4ef5-9f24-4135c3cdd9c3"/>
    <input alias="RobotInfo_topic" display="机器人位置姿态" name="RobotData" note="四轮车的模型接口" type="6fe0e80e-e781-4ff1-934f-de2cf07f2cc7"/>
   </inputs>
   <outputs>
    <output alias="ImageData_topic" display="图像数据配置" name="ImageData" note="输出图像数据" type="ae19c0f4-f801-404c-952f-aaff50572861"/>
    <output alias="LidarData_topic" display="激光数据配置" name="LidarData" note="输出激光数据" type="36f998e7-1ac3-4e24-95d1-1eb8a291071d"/>
    <output alias="RoadData_topic" display="tof相机数据" name="Tof" note="输出tof的相机数据" type="61cf8616-78ee-4d0e-b902-ad1bce69c76f"/>
   </outputs>
   <threads>
    <thread interval="" name="ControlRobot" note="控制机械臂或者指控相关的命令" sync_type="" timer_type=""/>
   </threads>
  </model>
 </models>
 <service>
  <info name="VirtualScence" type="TYPE_SERVICE" uuid="VirtualScence"/>
  <members/>
  <models>
   <model model="c1d4d6b4-d0ac-438d-8ba9-bc1a686a62ad"/>
  </models>
 </service>
</root>
` },
    { name: 'SlamPlannerControl.xml', content: `<?xml version="1.0" encoding="UTF-8"?>
<root type="SERVICE">
 <types>
  <type>
   <info alias="SimConfig（仿真参数配置）" create_time="" creator="" last_modify="" modifier="" name="SimConfig" note="仿真参数配置" type="TYPE_STRUCT" uuid="ef0734e0-e706-46ed-ae4e-c7e01923b4e8" version=""/>
   <member array="" display="real32" display_name="sim_step（仿真步长）" elem_type="--" name="sim_step" note="仿真步长" range="" sequence_type="" type="real32" unit_name="s"/>
   <member array="" display="int8" display_name="task(任务)【下拉菜单】" elem_type="--" name="task" note="陆域仿真执行的任务，决定启动的任务{定点排爆，0；边境巡逻，1}" range="" sequence_type="" type="int8" unit_name=""/>
   <member array="" display="int8" display_name="dection_alg(检测算法)【下拉菜单】" elem_type="--" name="dection_alg" note="目标检测算法，0：yolo8算法 1：yolo10算法" range="" sequence_type="" type="int8" unit_name=""/>
   <member array="" display="int8" display_name="slam_alg(定位建图算法)【下拉菜单】" elem_type="--" name="slam_alg" note="定位建图算法，0: lio_sam, 1: lidar_slam" range="" sequence_type="" type="int8" unit_name=""/>
  </type>
  <type>
   <info alias="SlamConfig（定位建图算法配置）" create_time="" creator="" last_modify="" modifier="" name="SlamConfig" note="Slam相关的配置参数" type="TYPE_STRUCT" uuid="89628f1a-8aaa-492f-9090-8122b32cc8d6" version=""/>
   <member array="" display="real32" display_name="acc_noise（加速度噪声）" elem_type="--" name="acc_noise" note="" range="" sequence_type="" type="real32" unit_name="m/s2"/>
   <member array="" display="real32" display_name="gry_noise(角速率噪声)" elem_type="--" name="gry_noise" note="" range="" sequence_type="" type="real32" unit_name="°/s"/>
   <member array="" display="real32" display_name="acc_bias(加速度偏置)" elem_type="--" name="acc_bias" note="" range="" sequence_type="" type="real32" unit_name="m/s2"/>
   <member array="" display="real32" display_name="gry_bias(角速率偏置)" elem_type="--" name="gry_bias" note="" range="" sequence_type="" type="real32" unit_name="°/s"/>
  </type>
  <type>
   <info alias="PlannerConfig（规划算法配置）" create_time="" creator="" last_modify="" modifier="" name="PlannerConfig" note="规划相关的配置参数" type="TYPE_STRUCT" uuid="982fe5bd-6093-4194-b754-cd893a00c996" version=""/>
   <member array="" display="real32" display_name="max_speed（最大速度）" elem_type="--" name="max_speed" note="规划输出的最大速度信息" range="" sequence_type="" type="real32" unit_name="m/s"/>
   <member array="" display="real32" display_name="max_yawrate（最大角速率）" elem_type="--" name="max_yawrate" note="最大转向角速率" range="" sequence_type="" type="real32" unit_name="rad/s"/>
   <member array="" display="Vector3" display_name="initial_position（初始位置）" elem_type="--" name="initial_position" note="初始位姿信息" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
   <member array="" display="int8" display_name="target_type(路线设置)【下拉菜单】" elem_type="--" name="target_type" note="路线的类型：0：自定义， 1：固定路线1，2：固定路线2" range="" sequence_type="" type="int8" unit_name=""/>
   <member array="" display="sequence" display_name="target_position（目标位置）" elem_type="Vector3" name="target_position" note="目标点信息列表" range="" sequence_type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Vector3" create_time="" creator="" last_modify="" modifier="" name="Vector" note="三维向量" type="TYPE_STRUCT" uuid="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" version=""/>
   <member array="" display="real32" display_name="x(x)" elem_type="--" name="x" note="x轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="y(y)" elem_type="--" name="y" note="y轴" range="" sequence_type="" type="real32" unit_name="m"/>
   <member array="" display="real32" display_name="z(z)" elem_type="--" name="z" note="z轴" range="" sequence_type="" type="real32" unit_name="m"/>
  </type>
  <type>
   <info alias="SensorConfig（传感器配置）" create_time="" creator="" last_modify="" modifier="" name="SensorConfig" note="传感器配置参数" type="TYPE_STRUCT" uuid="33fd8c78-4cae-4c1e-b8ad-44f5cabe3d87" version=""/>
   <member array="" display="ImuConfig" display_name="imu_config（Imu配置）" elem_type="" name="imu_config" note="imu相关的配置参数" range="" sequence_type="" type="53cc0abe-ee64-4d69-9d08-648e18e10b1a" unit_name=""/>
   <member array="" display="LidarConfig" display_name="lidar_config（Lidar配置）" elem_type="" name="lidar_config" note="lidar相关的配置参数" range="" sequence_type="" type="6462967a-6935-4c97-a209-e55dc3b4f6f6" unit_name=""/>
  </type>
  <type>
   <info alias="ImuConfig" create_time="" creator="" last_modify="" modifier="" name="ImuConfig" note="Imu的配置参数" type="TYPE_STRUCT" uuid="53cc0abe-ee64-4d69-9d08-648e18e10b1a" version=""/>
   <member array="" display="real32" display_name="acc_noise（加速度噪声）" elem_type="--" name="acc_noise" note="加速度噪声" range="" sequence_type="" type="real32" unit_name="m/s2"/>
   <member array="" display="real32" display_name="gry_noise（角速率噪声）" elem_type="--" name="gry_noise" note="角速率噪声" range="" sequence_type="" type="real32" unit_name="°/s"/>
   <member array="" display="real32" display_name="acc_bias(加速度偏置)" elem_type="--" name="acc_bias" note="加速度偏置" range="" sequence_type="" type="real32" unit_name="m/s2"/>
   <member array="" display="real32" display_name="gry_bias(角速率偏置)" elem_type="--" name="gry_bias" note="角速率偏置" range="" sequence_type="" type="real32" unit_name="°/s"/>
  </type>
  <type>
   <info alias="LidarConfig" create_time="" creator="" last_modify="" modifier="" name="LidarConfig" note="激光传感器配置参数" type="TYPE_STRUCT" uuid="6462967a-6935-4c97-a209-e55dc3b4f6f6" version=""/>
   <member array="" display="int32" display_name="weather_condition(天气条件)【下拉菜单】" elem_type="--" name="weather_condition" note="天气条件  {正常，0；1，雨；1，雪，2，雾}" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="adverse_level（恶劣程度）【下拉菜单】" elem_type="--" name="adverse_level" note="天气恶劣程度等级{一级，0； 二级，1；三级，2；四级，3；五级，4}" range="" sequence_type="" type="int32" unit_name=""/>
   <member array="" display="int32" display_name="fre_lidar（频率）" elem_type="--" name="fre_lidar" note="激光扫描频率" range="" sequence_type="" type="int32" unit_name="hz"/>
   <member array="" display="real32" display_name="scan_angle（激光扫描角度）" elem_type="--" name="scan_angle" note="激光扫描角度" range="" sequence_type="" type="real32" unit_name="°"/>
  </type>
  <type>
   <info alias="ImuData" create_time="" creator="" last_modify="" modifier="" name="IMUDATA" note="IMU的数据结构体信息" type="TYPE_STRUCT" uuid="9b4de777-49d4-44a0-9ba4-d480d9589b01" version=""/>
   <member array="" display="Header" display_name="header" elem_type="" name="header" note="数据header描述" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="Vector4" display_name="orientation" elem_type="" name="orientation" note="姿态四元数" range="" sequence_type="" type="074fc1e2-3d50-4e12-b3ea-15af58b2ecb4" unit_name=""/>
   <member array="" display="sequence" display_name="orientation_covariance" elem_type="real32" name="orientation_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
   <member array="" display="Vector" display_name="angular_velocity" elem_type="" name="angular_velocity" note="角速率值" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="rad/s"/>
   <member array="" display="sequence" display_name="angular_velocity_covariance" elem_type="real32" name="angular_velocity_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
   <member array="" display="Vector" display_name="linear_acceleration" elem_type="" name="linear_acceleration" note="加速度值" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="m/s2"/>
   <member array="" display="sequence" display_name="linear_acceleration_covariance" elem_type="real32" name="linear_acceleration_covariance" note="" range="" sequence_type="real32" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="Header" create_time="" creator="" last_modify="" modifier="" name="HEADER" note="header描述文件" type="TYPE_STRUCT" uuid="17f6811e-067e-4a7d-8ef4-ac528fe1f047" version=""/>
   <member array="" display="string" display_name="frame_id" elem_type="--" name="frame_id" note="数据所处的坐标系" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="Stamp" display_name="stamp" elem_type="" name="stamp" note="时间戳相关的信息" range="" sequence_type="" type="7fcc7a13-83bd-4fd4-91d8-026f7631c086" unit_name=""/>
  </type>
  <type>
   <info alias="Stamp" create_time="" creator="" last_modify="" modifier="" name="Stamp" note="主要用于记录时间戳相关的信息" type="TYPE_STRUCT" uuid="7fcc7a13-83bd-4fd4-91d8-026f7631c086" version=""/>
   <member array="" display="int32" display_name="sec" elem_type="--" name="sec" note="时间戳，表示秒的部分" range="" sequence_type="" type="int32" unit_name="秒"/>
   <member array="" display="uint32" display_name="nanosec" elem_type="--" name="nanosec" note="时间戳，表示纳秒的部分" range="" sequence_type="" type="uint32" unit_name="纳秒"/>
  </type>
  <type>
   <info alias="Vector4" create_time="" creator="" last_modify="" modifier="" name="Vector4" note="四维向量" type="TYPE_STRUCT" uuid="074fc1e2-3d50-4e12-b3ea-15af58b2ecb4" version=""/>
   <member array="" display="real32" display_name="字段1" elem_type="--" name="x" note="四维数x" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段2" elem_type="--" name="y" note="四维数y" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段3" elem_type="--" name="z" note="四维数z" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段4" elem_type="--" name="w" note="四维数w" range="" sequence_type="" type="real32" unit_name=""/>
  </type>
  <type>
   <info alias="LidarData" create_time="" creator="" last_modify="" modifier="" name="LidarData" note="激光传感器数据" type="TYPE_STRUCT" uuid="36f998e7-1ac3-4e24-95d1-1eb8a291071d" version=""/>
   <member array="" display="Header" display_name="字段1" elem_type="" name="header" note="激光传感器的帧头数据" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="sequence" display_name="字段2" elem_type="LidarPoint" name="points" note="激光传感器中的点相关信息" range="" sequence_type="a45f9442-1645-42b8-b26d-e63e975e5fe3" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="LidarPoint" create_time="" creator="" last_modify="" modifier="" name="LidarPoint" note="激光点数据结构体" type="TYPE_STRUCT" uuid="a45f9442-1645-42b8-b26d-e63e975e5fe3" version=""/>
   <member array="" display="real32" display_name="字段1" elem_type="--" name="x" note="位置x" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段2" elem_type="--" name="y" note="位置y" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段3" elem_type="--" name="z" note="位置z" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="uint8" display_name="字段4" elem_type="--" name="intensity" note="反射率" range="" sequence_type="" type="uint8" unit_name=""/>
   <member array="" display="uint16" display_name="字段5" elem_type="--" name="ring" note="通道" range="" sequence_type="" type="uint16" unit_name=""/>
   <member array="" display="real64" display_name="字段6" elem_type="--" name="time" note="时间" range="" sequence_type="" type="real64" unit_name=""/>
  </type>
  <type>
   <info alias="ImageData" create_time="" creator="" last_modify="" modifier="" name="ImageData" note="图像数据输入" type="TYPE_STRUCT" uuid="ae19c0f4-f801-404c-952f-aaff50572861" version=""/>
   <member array="" display="Header" display_name="字段1" elem_type="--" name="header" note="" range="" sequence_type="" type="17f6811e-067e-4a7d-8ef4-ac528fe1f047" unit_name=""/>
   <member array="" display="uint32" display_name="字段2" elem_type="--" name="height" note="" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="uint32" display_name="字段3" elem_type="--" name="width" note="" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="uint8" display_name="字段4" elem_type="--" name="is_bigendian" note="" range="" sequence_type="" type="uint8" unit_name=""/>
   <member array="" display="uint32" display_name="字段5" elem_type="--" name="step" note="每一行像素数据的字节数" range="" sequence_type="" type="uint32" unit_name=""/>
   <member array="" display="string" display_name="字段7" elem_type="--" name="encoding" note="指定图像数据的编码格式" range="" sequence_type="" type="string" unit_name=""/>
   <member array="" display="sequence" display_name="字段6" elem_type="uint8" name="data" note="图像数据存放位置" range="" sequence_type="uint8" type="sequence" unit_name=""/>
  </type>
  <type>
   <info alias="ControlSignal" create_time="" creator="" last_modify="" modifier="" name="ControlSignal" note="控制输出指令" type="TYPE_STRUCT" uuid="9232b8bd-8115-4ffa-8161-d55f307caa03" version=""/>
   <member array="" display="real32" display_name="yaw_rate" elem_type="--" name="yaw_rate" note="转向角速率" range="" sequence_type="" type="real32" unit_name="rad/s"/>
   <member array="" display="real32" display_name="velocity" elem_type="--" name="velocity" note="线速度" range="" sequence_type="" type="real32" unit_name="m/s"/>
  </type>
  <type>
   <info alias="ReachInfo" create_time="" creator="" last_modify="" modifier="" name="ReachInfo" note="目标点相关信息" type="TYPE_STRUCT" uuid="e8d9c354-39c3-4de4-be0a-f45bd30e2db8" version=""/>
   <member array="" display="real32" display_name="字段1" elem_type="--" name="start_time" note="任务开始时间" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="real32" display_name="字段2" elem_type="--" name="end_time" note="任务结束时间" range="" sequence_type="" type="real32" unit_name=""/>
   <member array="" display="Vector" display_name="字段3" elem_type="" name="target_point" note="目标点坐标" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name="m"/>
  </type>
 </types>
 <models>
  <model>
   <info address="" display_name="SlamPlannerControl" language="c++" name="SlamPlannerControl" note="" person="巫永舟" software="" type="TYPE_MODEL" uuid="fded0752-d35b-44fb-919c-5363aa9f2b6b" version="v1.0.0"/>
   <members>
    <member display="仿真配置" name="sim_config" note="初始化仿真相关的接口信息，以及配置参数" type="ef0734e0-e706-46ed-ae4e-c7e01923b4e8"/>
    <member display="slam配置" name="slam_config" note="定位建图算法配置" type="89628f1a-8aaa-492f-9090-8122b32cc8d6"/>
    <member display="planner配置" name="planner_config" note="规划控制算法参数配置" type="982fe5bd-6093-4194-b754-cd893a00c996"/>
    <member display="sensor配置" name="sensor_config" note="传感器参数配置" type="33fd8c78-4cae-4c1e-b8ad-44f5cabe3d87"/>
   </members>
   <inputs>
    <input alias="IMUDATA_topic" display="IMU数据" name="IMUData" note="从动力学模型订阅IMU的相关数据" type="9b4de777-49d4-44a0-9ba4-d480d9589b01"/>
    <input alias="LidarData_topic" display="LIDAR数据" name="LidarData" note="从UE中订阅LIDAR的相关数据" type="36f998e7-1ac3-4e24-95d1-1eb8a291071d"/>
    <input alias="ImageData_topic" display="Image数据" name="ImageData" note="主要用于目标检测所需要的相关信息" type="ae19c0f4-f801-404c-952f-aaff50572861"/>
   </inputs>
   <outputs>
    <output alias="ControlSignal_topic" display="控制信号" name="ControlCmd" note="输出控制指令给动力学模型" type="9232b8bd-8115-4ffa-8161-d55f307caa03"/>
    <output alias="ReachInfo_topic" display="任务完成信息" name="ReachInfo" note="任务完成相关的信息" type="e8d9c354-39c3-4de4-be0a-f45bd30e2db8"/>
    <output alias="Vector_topic" display="到达目的地标志" name="ReachGoal" note="到达目的地标志（已经不用了）" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2"/>
   </outputs>
   <threads>
    <thread interval="" name="run_0" note="description" sync_type="" timer_type=""/>
   </threads>
  </model>
 </models>
 <service>
  <info name="SlamPlannerControl" type="TYPE_SERVICE" uuid="SlamPlannerControl"/>
  <members/>
  <models>
   <model model="fded0752-d35b-44fb-919c-5363aa9f2b6b"/>
  </models>
 </service>
</root>
` },
  ]
  const nodes = await Promise.all(files.map(f => loadXMLFile(f.name, f.content)))
  treeData.value = nodes
}

// 拖拽模型节点开始
const onModelDragStart = (event: DragEvent, data: TreeNode) => {
  if (data.modelData) {
    event.stopPropagation()
    const json = JSON.stringify(data.modelData)
    event.dataTransfer?.setData('application/json', json)
    event.dataTransfer?.setData('text/plain', json)
    event.dataTransfer!.effectAllowed = 'copy'
    console.log('拖拽开始，数据：', data.modelData)
    emit('node-drag-start', data.modelData)
  }
}

onMounted(() => {
  initSampleFiles()
})
</script>

<style scoped>
.file-tree-parent {
  width: 100%;
  height: 100%;
  padding: 12px;
  box-sizing: border-box;
  background: #ffffff;
  border-radius: 12px;
  border: 1px solid rgba(0, 0, 0, 0.06);
  box-shadow: 0 1px 8px rgba(0, 0, 0, 0.06);
  overflow: hidden;
}
.file-tree-child {
  width: 100%;
  height: 100%;
  min-height: 0;
  background: transparent;
}
.upload-btn {
  margin-bottom: 16px;
}
.file-node, .model-node {
  display: inline-flex;
  align-items: center;
  gap: 6px;
}
.model-node {
  cursor: grab;
}
.model-node:active {
  cursor: grabbing;
}

h4 {
  margin: 0 0 12px 0;
  font-size: 14px;
  color: #303133;
}
</style>