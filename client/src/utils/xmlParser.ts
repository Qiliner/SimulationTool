// utils/xmlParser.ts
export interface ModelInput {
  name: string
  alias: string
  type: string
  note?: string
}

export interface ModelOutput {
  name: string
  alias: string
  type: string
  note?: string
}

export interface Type {
  name: string
  uuid: string
  typeElement?: string
}

export interface ModelMember {
  name: string
  display_name?: string
  type: string
  note?: string
  array?: string
  sequence_type?: string
  display_type?: string
  defaultValue?: any
  defaultSequence?: any[]
  typeInfo?: any
}

export interface ParsedModel {
  name: string
  displayName?: string
  uuid: string
  note?: string
  inputs: ModelInput[]
  outputs: ModelOutput[]
  members: ModelMember[]
  types?: Type[]
}

// 简易XML解析（实际项目可用DOMParser）
export function parseXMLToModels(xmlString: string): ParsedModel[] {
  const parser = new DOMParser()
  const doc = parser.parseFromString(xmlString, 'text/xml')
  const models: ParsedModel[] = []

  const typeNodes = doc.getElementsByTagName("type");
  const types: Type[] = [];

  const serializer = new XMLSerializer();

  for (let i = 0; i < typeNodes.length; i++) {
    const typeNode = typeNodes[i];
    if(!typeNode) continue
    const infoNode = typeNode.getElementsByTagName("info")[0];
    if (infoNode) {
      const name = infoNode.getAttribute("name");
      const uuid = infoNode.getAttribute("uuid");
      if (name && uuid) {
        const typeElement = serializer.serializeToString(typeNode);
        types.push({
          name: name,
          uuid: uuid,
          typeElement: typeElement
        });
      } 
    } 
  }

  const modelNodes = doc.getElementsByTagName('model')
  for (let i = 0; i < modelNodes.length; i++) {
    const modelEl = modelNodes.item(i)
    if (!modelEl) continue
    const infoEl = modelEl.getElementsByTagName('info').item(0)
     if (!infoEl) continue
    const name = infoEl?.getAttribute('name') || ''
    const displayName = infoEl?.getAttribute('display_name') || name
    const uuid = infoEl?.getAttribute('uuid') || ''
    const note = infoEl?.getAttribute('note') || ''

    // 解析inputs
    const inputs: ModelInput[] = []
    const inputNodes = modelEl.getElementsByTagName('input')
    for (let j = 0; j < inputNodes.length; j++) {
      const inp = inputNodes[j]
      if (!inp) continue
      inputs.push({
        name: inp.getAttribute('name') || '',
        alias: inp.getAttribute('alias') || '',
        type: inp.getAttribute('type') || '',
        note: inp.getAttribute('note') || '',
      })
    }

    // 解析outputs
    const outputs: ModelOutput[] = []
    const outputNodes = modelEl.getElementsByTagName('output')
    for (let j = 0; j < outputNodes.length; j++) {
      const out = outputNodes[j]
      if (!out) continue
      outputs.push({
        name: out.getAttribute('name') || '',
        alias: out.getAttribute('alias') || '',
        type: out.getAttribute('type') || '',
        note: out.getAttribute('note') || '',
      })
    }

    // 解析members
    const members: ModelMember[] = []
    const membersNode = modelEl.getElementsByTagName('members')[0]
    if (membersNode) {
      const memberNodes = membersNode.getElementsByTagName('member')
      for (let j = 0; j < memberNodes.length; j++) {
        const mem = memberNodes.item(j)
        if (!mem) continue
        members.push({
          name: mem.getAttribute('name') || '',
          display_name: mem.getAttribute('display') || '',
          type: mem.getAttribute('type') || '',
          note: mem.getAttribute('note') || '',
          array: mem.getAttribute('array') || '',
          sequence_type: mem.getAttribute('sequence_type') || '',
          display_type: mem.getAttribute('display') || '',
        })
      }
    }

    models.push({
      name,
      displayName,
      uuid,
      note,
      inputs,
      outputs,
      members,
      types,
    })
  }
  return models
}



/*解析初始化参数属性*/

import type { BasicType, FieldMeta, FieldMetaWithProps, MemberAttr, StructFieldMeta, TypeDefinition } from './types';

const basicTypes: Set<string> = new Set([
  'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32',
  'int64', 'uint64', 'real32', 'real64', 'bool', 'string'
]);

export function isBasicType(typeName: string): boolean {
  return basicTypes.has(typeName);
}

// 解析XML字符串，返回类型映射和根成员列表
export function parseXmlConfig(xmlStr: string): {
  typesMap: Map<string, TypeDefinition>;
  rootMembers: MemberAttr[];
  topLevelMembers: { name: string; type: string }[];
} {
 const parser = new DOMParser();
  const doc = parser.parseFromString(xmlStr, 'application/xml');
  const typesMap = new Map<string, TypeDefinition>();
  
  const typeNodes = doc.querySelectorAll('types > type');
  typeNodes.forEach((typeNode) => {
    const info = typeNode.querySelector('info');
    const uuid = info?.getAttribute('uuid') || '';
    const name = info?.getAttribute('name') || '';
    const members: MemberAttr[] = [];
    const memberNodes = typeNode.querySelectorAll('member');
    memberNodes.forEach((member) => {
      members.push({
        name: member.getAttribute('name') || '',
        display_name: member.getAttribute('display_name') || member.getAttribute('name') || '',
        type: member.getAttribute('type') || '',
        note: member.getAttribute('note') || '',
        array: member.getAttribute('array') || '',
        sequence_type: member.getAttribute('sequence_type') || '',
        elem_type: member.getAttribute('elem_type') || '',
        unit_name: member.getAttribute('unit_name') || '',
      });
    });
    typesMap.set(uuid, { uuid, name, members });
  });

  const modelNode = doc.querySelector('models > model');
  if (!modelNode) return { typesMap, rootMembers: [], topLevelMembers: [] };
  const membersNode = modelNode.querySelector('members');
  if (!membersNode) return { typesMap, rootMembers: [], topLevelMembers: [] };
  const rootMemberNodes = membersNode.querySelectorAll('member');
  const rootMembers: MemberAttr[] = [];
  const topLevelMembers: { name: string; type: string }[] = [];
  rootMemberNodes.forEach((member) => {
    const name = member.getAttribute('name') || '';
    const type = member.getAttribute('type') || '';
    topLevelMembers.push({ name, type });
    rootMembers.push({
      name,
      display_name: member.getAttribute('display_name') || name,
      type,
      note: member.getAttribute('note') || '',
      array: member.getAttribute('array') || '',
      sequence_type: member.getAttribute('sequence_type') || '',
      elem_type: member.getAttribute('elem_type') || '',
      unit_name: member.getAttribute('unit_name') || '',
    });
  });
  return { typesMap, rootMembers, topLevelMembers };
}

// 根据成员属性构建字段元数据
export function buildFieldMeta(member: MemberAttr, typesMap: Map<string, TypeDefinition>): FieldMeta {
  const typeName = member.type;
  const isFixedArray = member.array && member.array !== '';
  const isSequence = typeName === 'sequence';

  if (isSequence) {
    const elementTypeId = member.sequence_type || member.elem_type;
    if (!elementTypeId) throw new Error('sequence missing element type');
    let elementMeta: FieldMeta;
    if (isBasicType(elementTypeId)) {
      elementMeta = { kind: 'basic', typeName: elementTypeId as BasicType };
    } else {
      const structDef = typesMap.get(elementTypeId);
      if (structDef) elementMeta = buildStructMeta(structDef, typesMap);
      else elementMeta = { kind: 'basic', typeName: 'string' };
    }
    return { kind: 'sequence', typeName: 'sequence', elementMeta };
  }

  if (isFixedArray) {
    const arraySize = parseInt(member.array, 10);
    let elementMeta: FieldMeta;
    if (isBasicType(typeName)) {
      elementMeta = { kind: 'basic', typeName: typeName as BasicType };
    } else {
      const structDef = typesMap.get(typeName);
      if (structDef) elementMeta = buildStructMeta(structDef, typesMap);
      else elementMeta = { kind: 'basic', typeName: 'string' };
    }
    return { kind: 'fixedArray', typeName: 'array', arraySize, elementMeta };
  }

  if (isBasicType(typeName)) {
    return { kind: 'basic', typeName: typeName as BasicType };
  }

  const structDef = typesMap.get(typeName);
  if (structDef) {
    return buildStructMeta(structDef, typesMap);
  }
  // fallback
  return { kind: 'basic', typeName: 'string' };
}

// 构建结构体元数据（带字段名、展示名等）
export function buildStructMeta(structDef: TypeDefinition, typesMap: Map<string, TypeDefinition>): StructFieldMeta {
  const fields: FieldMetaWithProps[] = [];
  for (const member of structDef.members) {
    const fieldMeta = buildFieldMeta(member, typesMap);
    fields.push({
      ...fieldMeta,
      name: member.name,
      displayName: member.display_name || member.name,
      note: member.note,
      unit: member.unit_name,
    });
  }
  return {
    kind: 'struct',
    typeName: structDef.name,
    fields,
  };
}

// 生成默认值
export function getDefaultValue(meta: FieldMeta): any {
  if (meta.kind === 'basic') {
    const t = meta.typeName;
    if (t === 'bool') return false;
    if (t === 'string') return '';
    if (t.startsWith('int') || t.startsWith('uint')) return 0;
    if (t.startsWith('real')) return 0.0;
    return '';
  }
  if (meta.kind === 'struct') {
    const obj: Record<string, any> = {};
    for (const field of meta.fields) {
      obj[field.name] = getDefaultValue(field);
    }
    return obj;
  }
  if (meta.kind === 'fixedArray') {
    const arr = [];
    for (let i = 0; i < meta.arraySize; i++) {
      arr.push(getDefaultValue(meta.elementMeta));
    }
    return arr;
  }
  if (meta.kind === 'sequence') {
    return [];
  }
  return null;
}