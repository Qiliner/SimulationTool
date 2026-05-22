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