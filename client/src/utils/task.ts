import { request } from './request'

export async function createSimProject(data:any):Promise<any> {
  return request({
    url: "/api/tasks",
    method: 'POST',
    data,
  })
}

export async function getSimProjectList():Promise<any> {
  return request({
    url: "/api/tasks",
    method: 'GET'
  })
}

