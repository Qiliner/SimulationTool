
import { request } from './request'

export async function saveModelProjectData(url:string,id:string,data: any): Promise<any> {
  return request({
    url: url,
    method: 'POST',
    headers: {
        'X-Project-Id': id 
    },
    data,
  })
}

export async function generateModelCodel(data: any): Promise<any> {
    return request({
        url: '/generate',
        method: 'POST',
        headers: { 'Content-Type': 'application/xml' },
        data,
    })
}

export async function getModel(id:string): Promise<any>  {
    return request({    
        url: '/models',
        method: 'GET',
        headers: {  'X-Project-Id': id  },
    })
}

export async function getDataType(id:string) : Promise<any> {
    return request({    
        url: '/datatypes',
        method: 'GET',
        headers: {  'X-Project-Id': id  },
    })
}

export async function saveDataTypes(url:string,id:string,method:string,data: any): Promise<any> {
    return request({
        url: url,
        method: method,
        headers: { 'X-Project-Id': id  },
        data,
    })
}

export async function delDataTypes(url:string,id:string): Promise<any> {
    return request({
        url: url,
        method: "DELETE",
        headers: { 'X-Project-Id': id  }
    })
}

export async function saveModelDesign(url:string,id:string,method:string,data: any): Promise<any> {
    return request({
        url: url,
        method: method,
        headers: { 'X-Project-Id': id  },
        data,
    })
}

export async function delModelDesign(url:string,id:string): Promise<any> {
    return request({
        url: url,
        method: "DELETE",
        headers: { 'X-Project-Id': id  }
    })
}