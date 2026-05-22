
import { request } from './request'

interface LoginResponse {
  success: boolean
  user?: any
  token?: string
  error?: string
}

//登陆
export async function login(data: any): Promise<LoginResponse> {
  return request({
    url: '/login',
    method: 'POST',
    data,
  })
}

//获取用户信息
export async function getUserInfo(): Promise<any> {
  return request({
    url: "/users",
    method: 'GET',
  })
}

//添加用户
export async function saveUserInfo(url:string,method:string,data: any): Promise<any> {
  return request({
    url: url,
    method: method,
    data
  })
}

//删除用户
export async function delUserInfo(url:string,method:string): Promise<any> {
  return request({
    url: url,
    method: method
  })
}


export async function getProjectInfo(): Promise<any> {
  return request({
    url: "/projects",
    method: 'GET',
  })
}

export async function saveProjectInfo(url:string,method:string,data: any): Promise<any> {
  return request({
    url: url,
    method: method,
    data
  })
}

export async function delProjectInfo(url:string,method:string): Promise<any> {
  return request({
    url: url,
    method: method
  })
}
 

export async function delSoftWareInfo(url:string): Promise<any> {
  return request({
    url: url,
    method: 'DELETE'
  })
}

export async function downloadSoftWare(url:string): Promise<any> {
  return request({
    url: url
  })
}

export async function uploadSoftwareInfo(data:any): Promise<any> {
  return request({
    url: "/software",
    method: 'POST',
    data:data
  })
}

export async function moveModel(url:string,data:any): Promise<any> {
    return request({        
        url: url,   
        method: 'POST',
        data:data
    })
}

export async function delModel(url:string): Promise<any> {
  return request({
    url: url,
    method: 'DELETE'
  })
}

export async function downLoadModel(url:string): Promise<any> {
  return request({
    url: url,
    method: 'GET'
  })
}

export async function uploadModelInfo(data:any): Promise<any> {
    return request({
        url: "/model/files",
        method: 'POST',
        data:data
    })
}

export async function getSoftWareInfo(): Promise<any>  {
    return request({   
        url: "/software",
        method: 'GET'
    })
}

export async function getModelInfo(url:string): Promise<any> {
    return request({
        url: url,
        method: 'GET'
    })
}

export async function renameDir(url:string,data:any): Promise<any> {
    return request({
        url: url,
        method: 'PUT',
        data:data
    })
}

export async function delDir(url:string): Promise<any> { 
    return request({
        url: url,
        method: 'DELETE'
    })
}

export async function createDir(data:any): Promise<any> {
    return request({
        url: "/model/directories",
        method: 'POST',
        data:data
    })
}

export async function getDirInfo(url:string): Promise<any>  {
    return request({
        url: url,
        method: 'GET'
    })   
}