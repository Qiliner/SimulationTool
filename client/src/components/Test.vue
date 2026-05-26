<template>
  <div class="editor-container">
    <div class="editor-header">
      <h2>📐 Unreal 虚拟场景配置编辑器</h2>
      <div class="btn-group">
        <button class="btn btn-secondary" @click="reset">重置默认</button>
        <button class="btn btn-primary" @click="showConfig">📥 收集配置</button>
        <button class="btn btn-primary" @click="showXml">📄 导出 XML</button>
      </div>
    </div>
    <div class="config-tree">
      <div v-if="loading">解析配置中...</div>
      <div v-else-if="rootFields.length === 0">未找到 models 下的 member 配置项</div>
      <div v-else>
        <PropertyEditor
          v-for="field in rootFields"
          :key="field.path"
          :field-meta="field.meta"
          :value="field.value"
          :path="field.path"
          :label="field.label"
          :note="field.note"
          @update="handleUpdate"
        />
        <div class="code-output" v-if="showJson">
          <strong>📄 当前配置 JSON:</strong>
          <pre>{{ jsonOutput }}</pre>
        </div>
        <div class="code-output" v-if="showXmlBlock">
          <strong>📄 生成的 member_values XML:</strong>
          <pre>{{ xmlOutput }}</pre>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref } from 'vue';
import PropertyEditor from '@/components/PropertyEditor.vue';
import { useConfigEditor } from '@/stores/useConfigEditor';

const { loading, rootFields, updateValue, resetToDefault, collectConfig, generateMemberValuesXml } = useConfigEditor();
const showJson = ref(false);
const showXmlBlock = ref(false);
const jsonOutput = ref('');
const xmlOutput = ref('');

const handleUpdate = (event: { path: string; value: any }) => {
  updateValue(event);
};

const reset = () => {
  resetToDefault();
  showJson.value = false;
  showXmlBlock.value = false;
};

const showConfig = () => {
  const config = collectConfig();
  jsonOutput.value = JSON.stringify(config, null, 2);
  showJson.value = true;
  showXmlBlock.value = false;
};

const showXml = () => {
  xmlOutput.value = generateMemberValuesXml();
  showXmlBlock.value = true;
  showJson.value = false;
};

</script>

<style scoped>
.editor-container {
  background: white;
  border-radius: 16px;
  box-shadow: 0 8px 20px rgba(0,0,0,0.05);
  overflow: hidden;
  max-width: 1400px;
  margin: 0 auto;
}
.editor-header {
  background: #2c3e50;
  color: white;
  padding: 16px 24px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  gap: 12px;
}
.btn-group { display: flex; gap: 12px; }
.btn {
  padding: 8px 18px;
  border-radius: 40px;
  border: none;
  font-weight: 500;
  cursor: pointer;
  transition: 0.2s;
}
.btn-primary { background: #3498db; color: white; }
.btn-primary:hover { background: #2980b9; }
.btn-secondary { background: #ecf0f1; color: #2c3e50; }
.config-tree { padding: 20px 24px; max-height: calc(100vh - 180px); overflow-y: auto; }
.code-output {
  background: #1e2a3a;
  color: #e2e8f0;
  padding: 16px;
  border-radius: 12px;
  font-family: monospace;
  font-size: 12px;
  overflow-x: auto;
  margin-top: 20px;
}
</style>