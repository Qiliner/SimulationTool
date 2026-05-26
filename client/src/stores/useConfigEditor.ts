import { reactive, ref, readonly } from 'vue';
import type { FieldMeta, RootField, UpdateEvent } from '../utils/types';
import { parseXmlConfig, buildFieldMeta, getDefaultValue } from '../utils/xmlParser';

const fullXmlString = `<?xml version="1.0" encoding="UTF-8"?>
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
			<member array="" display="int32" display_name="id（-）" elem_type="--" name="id" note="编号" range="" sequence_type="" type="int32" unit_name=""/>
			<member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
			<member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
			<member array="" display="string" display_name="model_name（-）" elem_type="--" name="model_name" note="模型名称" range="" sequence_type="" type="string" unit_name=""/>
			<member array="" display="string" display_name="model_path（-）" elem_type="--" name="model_path" note="模型路径" range="" sequence_type="" type="string" unit_name=""/>
		</type>
		<type>
			<info alias="UnrealDynamicEntity" create_time="" creator="" last_modify="" modifier="" name="UnrealDynamicEntity" note="动态环境要素配置" type="TYPE_STRUCT" uuid="6a3bd2b2-bca3-4a36-a9a6-34759e61ce5c" version=""/>
			<member array="" display="string" display_name="type（模型类型）" elem_type="--" name="type" note="实体类型，如，车辆" range="" sequence_type="" type="string" unit_name=""/>
			<member array="" display="int32" display_name="id（-）" elem_type="--" name="id" note="编号" range="" sequence_type="" type="int32" unit_name=""/>
			<member array="" display="Vector3" display_name="location（位置）" elem_type="--" name="location" note="位置" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
			<member array="" display="Vector3" display_name="rotation（姿态）" elem_type="--" name="rotation" note="姿态" range="" sequence_type="" type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" unit_name=""/>
			<member array="" display="string" display_name="model_name（-）" elem_type="--" name="model_name" note="模型名称" range="" sequence_type="" type="string" unit_name=""/>
			<member array="" display="string" display_name="model_path（-）" elem_type="--" name="model_path" note="模型路径" range="" sequence_type="" type="string" unit_name=""/>
			<member array="" display="sequence" display_name="waypoints(运动路径)" elem_type="Vector3" name="waypoints" note="运动路径" range="" sequence_type="9631a8dd-6a2a-4b3c-9c88-725cc7bf90f2" type="sequence" unit_name=""/>
			<member array="" display="real32" display_name="speed(移动速度)" elem_type="--" name="speed" note="移动速度" range="" sequence_type="" type="real32" unit_name=""/>
		</type>
		<type>
			<info alias="UnrealRobotEntity" create_time="" creator="" last_modify="" modifier="" name="UnrealRobotEntity" note="Unreal的机器人配置" type="TYPE_STRUCT" uuid="fdea77e5-e359-4860-92db-eec161ec601c" version=""/>
			<member array="" display="UnrealSensorConfig" display_name="robot_sensor（机器人传感器）" elem_type="--" name="robot_sensor" note="" range="" sequence_type="" type="2d37530f-33db-4cb4-a84b-5f38317816db" unit_name=""/>
			<member array="" display="int32" display_name="robot_type（机器人类型）" elem_type="--" name="robot_type" note="机器人类型（不同类型的机器人设置）" range="" sequence_type="" type="int32" unit_name=""/>
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
			<info address="" display_name="VirtualRobotScence" language="C++，蓝图" name="VirtualRobotScence" note="" person="" software="UE5" type="TYPE_MODEL" uuid="7caaea26-cab3-4cb5-88ef-ba56aa32dd33" version=""/>
			<members>
				<member display="场景初始化需要的配置参数" name="Ueconfig" note="场景初始化需要的配置参数" type="6d71b57e-3bba-4ae2-9e34-b7b47fe664ad"/>
			</members>
			<inputs>
				<input alias="RobotInfo_topic" display="机器人位置姿态" name="RobotInfo" note="字符串流形式输入" type="6fe0e80e-e781-4ff1-934f-de2cf07f2cc7"/>
			</inputs>
			<outputs>
				<output alias="ImageData_topic" display="图像数据配置" name="ImageData" note="输出图像数据" type="ae19c0f4-f801-404c-952f-aaff50572861"/>
				<output alias="LidarData_topic" display="激光数据配置" name="LidarData" note="输出激光数据" type="36f998e7-1ac3-4e24-95d1-1eb8a291071d"/>
				<output alias="RoadData_topic" display="地形相机数据" name="RoadData" note="输出地形数据" type="61cf8616-78ee-4d0e-b902-ad1bce69c76f"/>
			</outputs>
			<threads>
				<thread interval="" name="ControlRobot" note="控制机械臂或者指控相关的命令" sync_type="" timer_type=""/>
			</threads>
		</model>
	</models>
	<service>
		<info name="VirtualRobotScence" type="TYPE_SERVICE" uuid="VirtualRobotScence"/>
		<members/>
		<models>
			<model model="7caaea26-cab3-4cb5-88ef-ba56aa32dd33"/>
		</models>
	</service>
</root>`;
// 注意：实际生产环境应将完整XML字符串放置于此，或从外部文件导入。

export function useConfigEditor() {
  const loading = ref(true);
  const rootFields = ref<RootField[]>([]);
  const topLevelTypeMap = ref<Record<string, string>>({}); // name -> uuid

  const buildConfig = () => {
    const { typesMap, rootMembers, topLevelMembers } = parseXmlConfig(fullXmlString);
    // 存储顶层类型映射
    const typeMap: Record<string, string> = {};
    for (const m of topLevelMembers) {
      typeMap[m.name] = m.type;
    }
    topLevelTypeMap.value = typeMap;

    const fields: RootField[] = [];
    for (const member of rootMembers) {
      const meta = buildFieldMeta(member, typesMap);
      const defaultValue = getDefaultValue(meta);
      fields.push({
        meta,
        value: reactive(defaultValue),
        path: member.name,
        label: member.display_name || member.name,
        note: member.note,
      });
    }
    rootFields.value = fields;
    loading.value = false;

  };

  const updateValue = ({ path, value }: UpdateEvent) => {
    // // 根据路径设置嵌套对象的值
    // const setValueByPath = (obj: any, pathStr: string, val: any) => {
    //   const keys = pathStr.split(/\.|\[|\]/).filter(k => k !== '');
    //   let current = obj;
    //   for (let i = 0; i < keys.length - 1; i++) {
    //     const key = keys[i];
    //     if (current[key] === undefined) current[key] = {};
    //     current = current[key];
    //   }
    //   const lastKey = keys[keys.length - 1];
    //   if (lastKey !== undefined) {
    //     current[lastKey] = val;
    //   }
    // };
    // for (const field of rootFields.value) {
    //   if (path === field.path || path.startsWith(field.path + '.') || path.startsWith(field.path + '[')) {
    //     const subPath = path.replace(field.path, '').replace(/^\./, '');
    //     setValueByPath(field.value, subPath, value);
    //     break;
    //   }
    // }
    // // 触发响应式更新
    // rootFields.value = [...rootFields.value];
	    for (const field of rootFields.value) {
      // 匹配当前字段的路径（精确匹配或作为前缀）
      if (path === field.path || path.startsWith(field.path + '.') || path.startsWith(field.path + '[')) {
        let subPath = path.slice(field.path.length);
        if (subPath.startsWith('.')) subPath = subPath.slice(1);
        
        // 情况1：直接更新顶层字段（例如整个 sequence 数组）
        if (subPath === '') {
          const target = field.value;
          if (Array.isArray(target)) {
            // 保持数组引用不变，使用 splice 替换全部内容
            target.splice(0, target.length, ...(Array.isArray(value) ? value : []));
          } else if (typeof target === 'object' && target !== null) {
            // 对象则合并属性
            Object.assign(target, value);
          } else {
            // 基本类型（很少出现，但防御处理）
            (field as any).value = value;
          }
        } 
        // 情况2：更新深层路径
        else {
          const setValueByPath = (obj: any, pathStr: string, val: any) => {
            const keys = pathStr.split(/\.|\[|\]/).filter(k => k !== '');
            let current = obj;
            for (let i = 0; i < keys.length - 1; i++) {
              const key = keys[i];
              if (current[key] === undefined) current[key] = {};
              current = current[key];
            }
            const lastKey = keys[keys.length - 1];
            if (lastKey !== undefined) {
              current[lastKey] = val;
            }
          };
          setValueByPath(field.value, subPath, value);
        }
        break;
      }
    }
  };

  const resetToDefault = () => {
    buildConfig();
  };

  const collectConfig = (): Record<string, any> => {
    const config: Record<string, any> = {};
    for (const field of rootFields.value) {
      config[field.path] = JSON.parse(JSON.stringify(field.value));
    }
    return config;
  };

  const generateValueNodes = (
    obj: any,
    meta: FieldMeta,
    currentPath: string,
    nodes: string[]
  ) => {
    if (meta.kind === 'basic') {
      let valueStr = '';
      if (obj === null || obj === undefined) valueStr = '';
      else if (typeof obj === 'boolean') valueStr = obj ? 'true' : 'false';
      else valueStr = String(obj);
      // XML 转义
      valueStr = valueStr.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
      nodes.push(`<value path="${currentPath}">${valueStr}</value>`);
    } 
    else if (meta.kind === 'struct') {
      for (const field of meta.fields) {
        const childValue = obj?.[field.name];
        const childPath = currentPath ? `${currentPath}.${field.name}` : field.name;
        generateValueNodes(childValue, field, childPath, nodes);
      }
    }
    else if (meta.kind === 'sequence' || meta.kind === 'fixedArray') {
      // 输出容器标签（无内容）
      nodes.push(`<value path="${currentPath}"/>`);
      if (Array.isArray(obj)) {
        for (let i = 0; i < obj.length; i++) {
          const itemPath = `${currentPath}[${i}]`;
          generateValueNodes(obj[i], meta.elementMeta, itemPath, nodes);
        }
      }
    }
  };

  const generateMemberValuesXml = (): string => {
    let xml = '<member_values>\n';
    for (const field of rootFields.value) {
      const typeUuid = topLevelTypeMap.value[field.path];
      if (!typeUuid) continue;
      xml += `  <value type="${typeUuid}" name="${field.path}">\n`;
      const innerNodes: string[] = [];
      // 顶层字段的 value 是一个对象，开始递归，初始 currentPath 为空（因为属性名会作为第一级路径）
      generateValueNodes(field.value, field.meta, '', innerNodes);
      for (const node of innerNodes) {
        xml += `    ${node}\n`;
      }
      xml += `  </value>\n`;
    }
    xml += '</member_values>';
    return xml;
  };

  buildConfig();

  return {
    loading: readonly(loading),
    rootFields: readonly(rootFields),
    updateValue,
    resetToDefault,
    collectConfig,
	generateMemberValuesXml
  };
}