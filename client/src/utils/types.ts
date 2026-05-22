import { type Node, type Edge } from '@vue-flow/core';

// XML解析后的基本成员结构
export interface XmlMember {
  name: string;
  display_name: string;
  type: string; // 可能是原始类型(real32)或UUID
  elem_type?: string;
  note?: string;
  unit_name?: string;
  sequenceData?: any[]; // 用于控制Sequence动态增删
}

// 节点的核心数据结构
export interface ModelData {
  uuid: string;
  name: string;
  displayName: string;
  inputs: Array<{ name: string; type: string; alias: string }>;
  outputs: Array<{ name: string; type: string; alias: string }>;
  members: XmlMember[];
}

// 扩展Vue Flow的Node类型
export type CustomNode = Node<ModelData>;

// 在线IP节点接口
export interface IpNode {
  ip: string;
  status: 'online' | 'offline';
  lastSeen: string;
}

// 自定义事件接口
export interface CustomEvent {
  id: string;
  name: string;
  content: string;
}