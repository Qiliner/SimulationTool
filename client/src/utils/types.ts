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



export type BasicType = 
  | 'int8' | 'uint8' | 'int16' | 'uint16'
  | 'int32' | 'uint32' | 'int64' | 'uint64'
  | 'real32' | 'real64'
  | 'bool' | 'string';

export interface BasicFieldMeta {
  kind: 'basic';
  typeName: BasicType;
}

export interface StructFieldMeta {
  kind: 'struct';
  typeName: string;
  fields: FieldMetaWithProps[];
}

export interface FixedArrayFieldMeta {
  kind: 'fixedArray';
  typeName: 'array';
  arraySize: number;
  elementMeta: FieldMeta;
}

export interface SequenceFieldMeta {
  kind: 'sequence';
  typeName: 'sequence';
  elementMeta: FieldMeta;
}

export type FieldMeta = BasicFieldMeta | StructFieldMeta | FixedArrayFieldMeta | SequenceFieldMeta;

export type FieldMetaWithProps = FieldMeta & {
  name: string;
  displayName: string;
  note?: string;
  unit?: string;
};

export interface TypeDefinition {
  uuid: string;
  name: string;
  members: MemberAttr[];
}

export interface MemberAttr {
  name: string;
  display_name: string;
  type: string;
  note: string;
  array: string;
  sequence_type: string;
  elem_type: string;
  unit_name: string;
}

export interface RootField {
  meta: FieldMeta;
  value: any;
  path: string;
  label: string;
  note: string;
}

export interface UpdateEvent {
  path: string;
  value: any;
}