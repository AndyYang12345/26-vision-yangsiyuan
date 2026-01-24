#!/bin/bash

# 自动提交脚本 - 完整功能版（含推送）

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 日志函数
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_step() { echo -e "${CYAN}➜${NC} $1"; }

# 配置变量
PUSH_TO_REMOTE=false
DRY_RUN=false
REMOTE_BRANCH=""
CURRENT_BRANCH=""

# 检查COMMITLOG.md
check_commitlog() {
    if [ ! -f "COMMITLOG.md" ]; then
        log_error "COMMITLOG.md不存在！"
        log_info "正在创建COMMITLOG.md模板..."
        cat > COMMITLOG.md << 'EOF'
## [0.0.1] - $(date +%Y-%m-%d)

### Feature 新增

+ 

### Changed 变更

* 

### Fixed 修复

* 
EOF
        log_info "请编辑COMMITLOG.md后重新运行脚本"
        exit 1
    fi
    
    if [ ! -s "COMMITLOG.md" ]; then
        log_error "COMMITLOG.md为空！"
        exit 1
    fi
}

# 获取当前分支
get_current_branch() {
    CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "")
    if [ -z "$CURRENT_BRANCH" ]; then
        log_error "当前目录不是Git仓库或无法获取分支信息"
        exit 1
    fi
    log_info "当前分支: $CURRENT_BRANCH"
    
    # 设置远程分支（默认为当前分支）
    REMOTE_BRANCH=${REMOTE_BRANCH:-$CURRENT_BRANCH}
}

# 解析版本号
parse_version() {
    local version_line=$(head -n 1 "COMMITLOG.md")
    if [[ "$version_line" =~ ^##\ \[([0-9]+\.[0-9]+\.[0-9]+) ]]; then
        VERSION="${BASH_REMATCH[1]}"
        log_info "版本号: $VERSION"
        
        # 解析日期
        if [[ "$version_line" =~ -[[:space:]]*([0-9]{4}-[0-9]{2}-[0-9]{2}) ]]; then
            VERSION_DATE="${BASH_REMATCH[1]}"
        else
            VERSION_DATE=$(date +"%Y-%m-%d")
        fi
        log_info "版本日期: $VERSION_DATE"
        return 0
    else
        log_error "第一行必须包含版本号，格式: ## [x.x.x] - yyyy-mm-dd"
        exit 1
    fi
}

# 解析COMMITLOG内容
parse_commitlog_content() {
    log_info "正在解析COMMITLOG.md内容..."
    
    # 初始化变量
    COMMIT_TITLE="Release v$VERSION"
    FEATURE_CONTENT=""
    CHANGED_CONTENT=""
    FIXED_CONTENT=""
    OPTIMIZATION_CONTENT=""
    DOCS_CONTENT=""
    OTHER_CONTENT=""
    COMMIT_BODY=""
    
    local current_section=""
    
    # 逐行读取
    while IFS= read -r line; do
        # 检测分类标题
        if [[ "$line" =~ ^###[[:space:]]+(Feature[[:space:]]+新增) ]]; then
            current_section="feature"
            continue
        elif [[ "$line" =~ ^###[[:space:]]+(Changed[[:space:]]+变更) ]]; then
            current_section="changed"
            continue
        elif [[ "$line" =~ ^###[[:space:]]+(Fixed[[:space:]]+修复) ]]; then
            current_section="fixed"
            continue
        elif [[ "$line" =~ ^###[[:space:]]+(Optimization[[:space:]]+优化) ]]; then
            current_section="optimization"
            continue
        elif [[ "$line" =~ ^###[[:space:]]+(Documentation[[:space:]]+文档) ]]; then
            current_section="docs"
            continue
        elif [[ "$line" =~ ^###[[:space:]]+(Other[[:space:]]+其他) ]]; then
            current_section="other"
            continue
        fi
        
        # 处理列表项
        if [[ "$line" =~ ^[+\*-][[:space:]]+(.+) ]]; then
            local item="${BASH_REMATCH[1]}"
            case "$current_section" in
                "feature")
                    FEATURE_CONTENT+="+ $item\n"
                    COMMIT_BODY+="• 新增: $item\n"
                    ;;
                "changed")
                    CHANGED_CONTENT+="* $item\n"
                    COMMIT_BODY+="• 变更: $item\n"
                    ;;
                "fixed")
                    FIXED_CONTENT+="* $item\n"
                    COMMIT_BODY+="• 修复: $item\n"
                    ;;
                "optimization")
                    OPTIMIZATION_CONTENT+="+ $item\n"
                    COMMIT_BODY+="• 优化: $item\n"
                    ;;
                "docs")
                    DOCS_CONTENT+="* $item\n"
                    COMMIT_BODY+="• 文档: $item\n"
                    ;;
                "other")
                    OTHER_CONTENT+="+ $item\n"
                    COMMIT_BODY+="• 其他: $item\n"
                    ;;
            esac
        fi
    done < "COMMITLOG.md"
    
    # 检查是否有内容
    if [ -z "$FEATURE_CONTENT$CHANGED_CONTENT$FIXED_CONTENT$OPTIMIZATION_CONTENT$DOCS_CONTENT$OTHER_CONTENT" ]; then
        log_error "没有找到任何有效的更新内容！"
        log_info "请在COMMITLOG.md中添加以+或*开头的列表项"
        exit 1
    fi
    
    log_info "解析完成"
}

# 更新CHANGELOG.md（在提交前更新）
update_changelog() {
    log_step "步骤1: 更新CHANGELOG.md"
    
    # 构建更新日志条目
    local changelog_entry="## [$VERSION] - $VERSION_DATE\n\n"
    
    # 添加Feature部分
    if [ -n "$FEATURE_CONTENT" ]; then
        changelog_entry+="### Feature 新增\n\n"
        changelog_entry+="$FEATURE_CONTENT\n"
    fi
    
    # 添加Changed部分
    if [ -n "$CHANGED_CONTENT" ]; then
        changelog_entry+="### Changed 变更\n\n"
        changelog_entry+="$CHANGED_CONTENT\n"
    fi
    
    # 添加Fixed部分
    if [ -n "$FIXED_CONTENT" ]; then
        changelog_entry+="### Fixed 修复\n\n"
        changelog_entry+="$FIXED_CONTENT\n"
    fi
    
    # 添加Optimization部分
    if [ -n "$OPTIMIZATION_CONTENT" ]; then
        changelog_entry+="### Optimization 优化\n\n"
        changelog_entry+="$OPTIMIZATION_CONTENT\n"
    fi
    
    # 添加Documentation部分
    if [ -n "$DOCS_CONTENT" ]; then
        changelog_entry+="### Documentation 文档\n\n"
        changelog_entry+="$DOCS_CONTENT\n"
    fi
    
    # 添加Other部分
    if [ -n "$OTHER_CONTENT" ]; then
        changelog_entry+="### Other 其他\n\n"
        changelog_entry+="$OTHER_CONTENT\n"
    fi
    
    changelog_entry+="\n"
    
    if [ "$DRY_RUN" = false ]; then
        # 创建或更新CHANGELOG.md
        if [ ! -f "CHANGELOG.md" ]; then
            log_info "创建新的CHANGELOG.md"
            echo -e "# 更新日志\n\n$changelog_entry" > CHANGELOG.md
        else
            log_info "更新现有的CHANGELOG.md"
            # 在开头插入新版本（跳过标题行）
            local temp_file=$(mktemp)
            head -n 1 CHANGELOG.md > "$temp_file"
            echo -e "\n$changelog_entry" >> "$temp_file"
            tail -n +2 CHANGELOG.md >> "$temp_file" 2>/dev/null || true
            mv "$temp_file" CHANGELOG.md
        fi
        
        log_success "CHANGELOG.md已更新"
    else
        log_info "[DRY RUN] 将更新CHANGELOG.md"
    fi
}

# 执行Git提交
git_commit() {
    log_step "步骤2: 执行Git提交"
    
    if [ "$DRY_RUN" = true ]; then
        log_info "[DRY RUN] 将执行: git add ."
        log_info "[DRY RUN] 将执行: git commit -m \"$COMMIT_TITLE\" -m \"...\""
        return 0
    fi
    
    # 检查是否有未提交的更改
    local changes_exist=false
    if ! git diff --quiet || ! git diff --cached --quiet; then
        changes_exist=true
    fi
    
    if [ "$changes_exist" = false ]; then
        log_warning "没有检测到需要提交的更改"
        read -p "是否继续提交？（仅提交CHANGELOG更新）(y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log_info "提交已取消"
            exit 0
        fi
    fi
    
    # 添加所有更改（包括新生成的CHANGELOG.md）
    git add .
    
    # 构建提交信息
    if [ -n "$COMMIT_BODY" ]; then
        # 将多行提交信息转换为单行摘要用于标题
        local first_line=$(echo "$COMMIT_BODY" | head -n 1 | sed 's/^• //')
        git commit -m "$COMMIT_TITLE: $first_line" -m "$COMMIT_BODY"
    else
        git commit -m "$COMMIT_TITLE"
    fi
    
    if [ $? -eq 0 ]; then
        log_success "Git提交成功"
        
        # 显示提交信息
        local last_commit=$(git log -1 --oneline)
        log_info "提交ID: $last_commit"
    else
        log_error "Git提交失败"
        exit 1
    fi
}

# 推送到远程仓库
push_to_remote() {
    log_step "步骤3: 推送到远程仓库"
    
    if [ "$PUSH_TO_REMOTE" = false ]; then
        log_info "跳过推送（未指定-p参数）"
        return 0
    fi
    
    if [ "$DRY_RUN" = true ]; then
        log_info "[DRY RUN] 将执行: git push origin $REMOTE_BRANCH"
        return 0
    fi
    
    # 检查远程仓库
    local remote_exists=$(git remote)
    if [ -z "$remote_exists" ]; then
        log_error "没有配置远程仓库"
        log_info "请先添加远程仓库: git remote add origin <url>"
        exit 1
    fi
    
    # 获取远程仓库名称（通常为origin）
    local remote_name="origin"
    
    log_info "正在推送到 $remote_name/$REMOTE_BRANCH ..."
    
    # 执行推送
    if git push "$remote_name" "$REMOTE_BRANCH"; then
        log_success "推送成功"
        
        # 显示推送信息
        local push_info=$(git log --oneline -1)
        log_info "已推送: $push_info"
        log_info "远程分支: $remote_name/$REMOTE_BRANCH"
    else
        log_error "推送失败"
        log_info "尝试使用: git push --set-upstream $remote_name $REMOTE_BRANCH"
        
        read -p "是否设置上游分支并推送？(y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            if git push --set-upstream "$remote_name" "$REMOTE_BRANCH"; then
                log_success "推送成功（已设置上游分支）"
            else
                log_error "推送失败，请手动检查"
                exit 1
            fi
        else
            log_info "推送已取消，请手动推送"
        fi
    fi
}

# 清理COMMITLOG.md（在提交后清理）
cleanup_commitlog() {
    log_step "步骤4: 清理COMMITLOG.md"
    
    if [ "$DRY_RUN" = true ]; then
        log_info "[DRY RUN] 将重置COMMITLOG.md为下一个版本模板"
        return 0
    fi
    
    # 计算下一个版本
    local next_version="0.0.2"
    if [[ "$VERSION" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
        local major="${BASH_REMATCH[1]}"
        local minor="${BASH_REMATCH[2]}"
        local patch="${BASH_REMATCH[3]}"
        next_version="$major.$minor.$((patch + 1))"
    fi
    
    # 创建新的COMMITLOG.md模板
    cat > COMMITLOG.md << EOF
## [$next_version] - $(date +%Y-%m-%d)

### Feature 新增

+ 

### Changed 变更

* 

### Fixed 修复

* 

### Optimization 优化

+ 

### Documentation 文档

* 

### Other 其他

+ 

# ------------------------------------------------------------------
# 填写说明：
# 1. 在对应分类下添加更新内容，使用+或*开头
# 2. 不需要的分类可以留空
# 3. 保存后运行: $0 [-p] [-b 分支名]
# ------------------------------------------------------------------
EOF
    
    log_success "COMMITLOG.md已重置"
    log_info "下次提交版本: $next_version"
}

# 显示提交预览
show_preview() {
    echo ""
    echo "══════════════════════════════════════════════════════════"
    echo -e "${CYAN}提交信息预览${NC}"
    echo "══════════════════════════════════════════════════════════"
    echo -e "${YELLOW}标题:${NC} $COMMIT_TITLE"
    echo -e "${YELLOW}版本:${NC} $VERSION ($VERSION_DATE)"
    echo -e "${YELLOW}分支:${NC} $CURRENT_BRANCH"
    echo ""
    echo -e "${YELLOW}更新内容:${NC}"
    
    if [ -n "$FEATURE_CONTENT" ]; then
        echo -e "${GREEN}新增功能:${NC}"
        echo -e "$FEATURE_CONTENT" | sed 's/^/  /'
    fi
    if [ -n "$CHANGED_CONTENT" ]; then
        echo -e "${BLUE}变更内容:${NC}"
        echo -e "$CHANGED_CONTENT" | sed 's/^/  /'
    fi
    if [ -n "$FIXED_CONTENT" ]; then
        echo -e "${RED}修复问题:${NC}"
        echo -e "$FIXED_CONTENT" | sed 's/^/  /'
    fi
    if [ -n "$OPTIMIZATION_CONTENT" ]; then
        echo -e "${CYAN}优化内容:${NC}"
        echo -e "$OPTIMIZATION_CONTENT" | sed 's/^/  /'
    fi
    if [ -n "$DOCS_CONTENT" ]; then
        echo -e "${BLUE}文档更新:${NC}"
        echo -e "$DOCS_CONTENT" | sed 's/^/  /'
    fi
    if [ -n "$OTHER_CONTENT" ]; then
        echo "其他更新:"
        echo -e "$OTHER_CONTENT" | sed 's/^/  /'
    fi
    
    echo ""
    echo -e "${YELLOW}操作流程:${NC}"
    echo "  1. 更新CHANGELOG.md"
    echo "  2. Git提交"
    if [ "$PUSH_TO_REMOTE" = true ]; then
        echo "  3. 推送到远程分支: $REMOTE_BRANCH"
    fi
    echo "  4. 清理COMMITLOG.md"
    
    if [ "$DRY_RUN" = true ]; then
        echo ""
        echo -e "${YELLOW}[DRY RUN 模式]${NC} 仅显示预览，不执行实际操作"
    fi
    echo "══════════════════════════════════════════════════════════"
    echo ""
}

# 显示帮助信息
show_help() {
    echo "自动提交脚本 v2.0"
    echo ""
    echo "用法: $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -h, --help           显示此帮助信息"
    echo "  -p, --push           提交后推送到远程仓库"
    echo "  -d, --dry-run        仅显示预览，不执行实际操作"
    echo "  -b, --branch NAME    指定远程分支名（默认: 当前分支）"
    echo "  -m, --message MSG    自定义提交标题（可选）"
    echo ""
    echo "示例:"
    echo "  $0                   仅提交到本地"
    echo "  $0 -p                提交并推送到远程"
    echo "  $0 -p -b main        提交并推送到远程main分支"
    echo "  $0 -d                仅预览提交内容"
    echo ""
    echo "工作流程:"
    echo "  1. 读取COMMITLOG.md → 2. 更新CHANGELOG.md"
    echo "  3. Git提交 → 4. [可选]推送 → 5. 清理COMMITLOG.md"
}

# 解析命令行参数
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -p|--push)
                PUSH_TO_REMOTE=true
                shift
                ;;
            -d|--dry-run)
                DRY_RUN=true
                shift
                ;;
            -b|--branch)
                if [ -n "$2" ]; then
                    REMOTE_BRANCH="$2"
                    shift 2
                else
                    log_error "-b 参数需要指定分支名"
                    exit 1
                fi
                ;;
            -m|--message)
                if [ -n "$2" ]; then
                    CUSTOM_TITLE="$2"
                    shift 2
                else
                    log_error "-m 参数需要指定消息"
                    exit 1
                fi
                ;;
            *)
                log_error "未知参数: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# 主函数
main() {
    log_info "🚀 开始自动提交流程..."
    
    # 解析命令行参数
    parse_arguments "$@"
    
    # 获取当前分支
    get_current_branch
    
    # 检查COMMITLOG.md
    check_commitlog
    
    # 解析版本号
    parse_version
    
    # 解析提交内容
    parse_commitlog_content
    
    # 自定义标题（如果有）
    if [ -n "$CUSTOM_TITLE" ]; then
        COMMIT_TITLE="$CUSTOM_TITLE"
    fi
    
    # 显示预览
    show_preview
    
    # 确认是否继续（非dry-run模式）
    if [ "$DRY_RUN" = false ]; then
        read -p "是否继续执行？(y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            log_info "操作已取消"
            exit 0
        fi
    else
        log_info "[DRY RUN] 模式，仅显示预览"
    fi
    
    echo ""
    echo "══════════════════════════════════════════════════════════"
    echo -e "${CYAN}开始执行...${NC}"
    echo "══════════════════════════════════════════════════════════"
    echo ""
    
    # 执行流程
    update_changelog
    echo ""
    
    git_commit
    echo ""
    
    push_to_remote
    echo ""
    
    cleanup_commitlog
    echo ""
    
    # 完成信息
    echo "══════════════════════════════════════════════════════════"
    log_success "✅ 所有操作已完成！"
    echo ""
    echo -e "${GREEN}总结:${NC}"
    echo "  • 版本: v$VERSION"
    echo "  • 分支: $CURRENT_BRANCH"
    echo "  • 提交: $(git log -1 --oneline 2>/dev/null || echo 'N/A')"
    
    if [ "$PUSH_TO_REMOTE" = true ]; then
        echo "  • 推送: 已推送到远程仓库"
    else
        echo "  • 推送: 未推送（使用 -p 参数推送）"
    fi
    
    echo "  • 日志: 已更新 CHANGELOG.md"
    echo "  • 模板: 已重置 COMMITLOG.md (v$next_version)"
    echo ""
    
    if [ "$PUSH_TO_REMOTE" = false ] && [ "$DRY_RUN" = false ]; then
        log_info "提示: 使用 '$0 -p' 推送到远程仓库"
    fi
    echo "══════════════════════════════════════════════════════════"
}

# 运行主函数
main "$@"