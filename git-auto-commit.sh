#!/bin/bash

# 自动提交脚本 - 修复版

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 日志函数
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# 检查COMMITLOG.md
check_commitlog() {
    if [ ! -f "COMMITLOG.md" ] || [ ! -s "COMMITLOG.md" ]; then
        log_error "COMMITLOG.md不存在或为空！"
        exit 1
    fi
}

# 解析版本号 - 简化版
parse_version() {
    local version_line=$(head -n 1 "COMMITLOG.md")
    if [[ "$version_line" =~ ^##\ \[([0-9]+\.[0-9]+\.[0-9]+) ]]; then
        VERSION="${BASH_REMATCH[1]}"
        log_info "版本号: $VERSION"
        return 0
    else
        log_error "第一行必须包含版本号，格式: ## [x.x.x]"
        exit 1
    fi
}

# 解析COMMITLOG内容 - 简化版
parse_commitlog_content() {
    log_info "正在解析COMMITLOG.md..."
    
    # 使用更简单的方法解析
    local in_feature=false
    local feature_content=""
    
    while IFS= read -r line; do
        # 检查是否是Feature分类
        if [[ "$line" == "### Feature 新增" ]]; then
            in_feature=true
            continue
        fi
        
        # 如果进入了Feature分类，收集内容
        if [ "$in_feature" = true ]; then
            # 如果遇到新的分类标题，停止收集
            if [[ "$line" =~ ^###[[:space:]]+ ]]; then
                break
            fi
            
            # 收集以+或*开头的内容
            if [[ "$line" =~ ^[+*][[:space:]]+(.+) ]]; then
                feature_content+="${BASH_REMATCH[1]}\n"
            fi
        fi
    done < "COMMITLOG.md"
    
    # 如果没有找到内容，尝试直接提取所有列表项
    if [ -z "$feature_content" ]; then
        log_warning "未在'Feature 新增'分类下找到内容，尝试直接提取..."
        feature_content=$(grep -E '^[+*]' "COMMITLOG.md" | sed 's/^[+*][[:space:]]*//' | tr '\n' ';')
    fi
    
    if [ -z "$feature_content" ]; then
        log_error "未找到任何提交内容！"
        exit 1
    fi
    
    # 设置提交信息
    COMMIT_TITLE="Release v$VERSION"
    COMMIT_BODY=$(echo -e "$feature_content" | sed 's/;$//' | tr ';' '\n')
    
    log_info "提交标题: $COMMIT_TITLE"
    log_info "提交内容: $COMMIT_BODY"
}

# 更新CHANGELOG.md
update_changelog() {
    log_info "更新CHANGELOG.md..."
    
    local today=$(date +"%Y-%m-%d")
    local changelog_entry="## [$VERSION] - $today\n\n"
    
    # 添加Feature分类
    changelog_entry+="### Feature 新增\n\n"
    echo -e "$COMMIT_BODY" | while IFS= read -r line; do
        if [ -n "$line" ]; then
            changelog_entry+="+ $line\n"
        fi
    done
    changelog_entry+="\n"
    
    # 添加到CHANGELOG.md
    if [ ! -f "CHANGELOG.md" ]; then
        echo -e "# 更新日志\n\n$changelog_entry" > CHANGELOG.md
    else
        # 在开头插入
        local temp_file=$(mktemp)
        echo -e "# 更新日志\n\n$changelog_entry" > "$temp_file"
        tail -n +2 CHANGELOG.md >> "$temp_file" 2>/dev/null || true
        mv "$temp_file" CHANGELOG.md
    fi
    
    log_success "CHANGELOG.md已更新"
}

# 清理COMMITLOG.md
cleanup_commitlog() {
    log_info "清理COMMITLOG.md..."
    
    # 计算下一个版本
    local next_version="0.0.2"
    if [[ "$VERSION" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
        local patch=$(( ${BASH_REMATCH[3]} + 1 ))
        next_version="${BASH_REMATCH[1]}.${BASH_REMATCH[2]}.$patch"
    fi
    
    # 创建新的COMMITLOG.md
    cat > COMMITLOG.md << EOF
## [$next_version] - $(date +%Y-%m-%d)

### Feature 新增

+ 

### Changed 变更

* 

### Fixed 修复

* 
EOF
    
    log_success "COMMITLOG.md已重置"
}

# 执行Git提交
git_commit() {
    log_info "执行Git提交..."
    
    if git diff --quiet && git diff --cached --quiet; then
        log_warning "没有需要提交的更改"
        return 0
    fi
    
    git add .
    
    if [ -n "$COMMIT_BODY" ]; then
        # 处理多行提交信息
        echo -e "$COMMIT_BODY" | while IFS= read -r line; do
            if [ -n "$line" ]; then
                git commit -m "$COMMIT_TITLE" -m "$line"
                break
            fi
        done
    else
        git commit -m "$COMMIT_TITLE"
    fi
    
    if [ $? -eq 0 ]; then
        log_success "Git提交成功"
    else
        log_error "Git提交失败"
        exit 1
    fi
}

# 主函数
main() {
    log_info "开始自动提交..."
    
    # 检查文件
    check_commitlog
    
    # 解析版本
    parse_version
    
    # 解析内容
    parse_commitlog_content
    
    echo ""
    echo "========================================"
    echo "提交信息预览:"
    echo "版本: v$VERSION"
    echo "标题: $COMMIT_TITLE"
    echo "内容:"
    echo "$COMMIT_BODY"
    echo "========================================"
    echo ""
    
    read -p "是否继续？(y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_info "操作取消"
        exit 0
    fi
    
    # 执行提交
    git_commit
    
    # 更新CHANGELOG
    update_changelog
    
    # 清理COMMITLOG
    cleanup_commitlog
    
    log_success "所有操作完成！"
}

main "$@"