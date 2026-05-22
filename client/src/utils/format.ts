// 安全地格式化数字
export const formatNumber = (value: any, decimals: number = 1): number => {
  if (value === undefined || value === null) return 0
  const num = parseFloat(value)
  return isNaN(num) ? 0 : parseFloat(num.toFixed(decimals))
}

export const getStatusText = (status: string) => {
  const map: Record<string, string> = { online: '在线', offline: '离线', busy: '繁忙' }
  return map[status] || status
}

export const formatTime = (timestamp: number) => {
  if (!timestamp) return '未知'
  return new Date(timestamp * 1000).toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  })
}

export const getProcessWidth = (memoryMB: any, totalGB: any = 16): number => {
  const mb = formatNumber(memoryMB)
  const totalMB = formatNumber(totalGB) * 1024
  if (totalMB === 0) return 0
  return Math.min((mb / totalMB) * 100, 100)
}

  // 安全地处理数值数据
export const safeNumber = (val: any, defaultVal: number = 0): number => {
    const num = parseFloat(val)
    return isNaN(num) ? defaultVal : num
}
  
export const formatDate = (timestamp: string) => {
  if (!timestamp || timestamp === '0') return '-'
  const date = new Date(parseInt(timestamp) * 1000)
  return date.toLocaleDateString('zh-CN')
}

export const formatFileSize = (bytes: number) => {
  if (!bytes) return '-'
  const sizes = ['B', 'KB', 'MB', 'GB']
  const i = Math.floor(Math.log(bytes) / Math.log(1024))
  return (bytes / Math.pow(1024, i)).toFixed(2) + ' ' + sizes[i]
}