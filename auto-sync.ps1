# Cprog 自动同步脚本
# 监控文件修改并自动推送到 GitHub

$git = "D:\Tools\Git\bin\git.exe"
$projectPath = "D:\我的项目\Cprog"
$syncInterval = 30  # 同步间隔（秒）

# 文件监控配置
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $projectPath
$watcher.IncludeSubdirectories = $true
$watcher.EnableRaisingEvents = $true

# 需要监控的文件类型
$watchExtensions = @('.c', '.h', '.cpp', '.hpp', '.cs', '.py', '.java', '.txt', '.md', '.slnx', '.sln', '.vcxproj', '.filters')

# 忽略的文件/目录
$ignorePatterns = @('.git', '.vs', 'x64', 'Debug', 'Release', '*.obj', '*.exe', '*.dll', '*.pdb')

$script:hasChanges = $false
$script:lastSync = Get-Date

function ShouldIgnore($path) {
    foreach ($pattern in $ignorePatterns) {
        if ($path -like "*$pattern*") { return $true }
    }
    return $false
}

function Sync-ToGitHub {
    Set-Location $projectPath
    
    # 检查是否有更改
    & $git status --porcelain | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[$(Get-Date -Format 'HH:mm:ss')] Git 状态检查失败" -ForegroundColor Red
        return
    }
    
    $status = & $git status --porcelain
    if ([string]::IsNullOrWhiteSpace($status)) {
        return  # 没有更改
    }
    
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] 检测到更改，正在同步..." -ForegroundColor Cyan
    
    # 添加所有更改
    & $git add -A
    
    # 提交
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    & $git commit -m "Auto sync: $timestamp" --quiet
    
    if ($LASTEXITCODE -eq 0) {
        # 推送到 GitHub
        & $git push origin main --quiet
        if ($LASTEXITCODE -eq 0) {
            Write-Host "[$(Get-Date -Format 'HH:mm:ss')] ✅ 同步成功！" -ForegroundColor Green
        } else {
            Write-Host "[$(Get-Date -Format 'HH:mm:ss')] ❌ 推送失败，请检查网络连接" -ForegroundColor Red
        }
    }
    
    $script:lastSync = Get-Date
    $script:hasChanges = $false
}

# 注册文件变更事件
Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action {
    $path = $Event.SourceEventArgs.FullPath
    $ext = [System.IO.Path]::GetExtension($path)
    
    if (ShouldIgnore $path) { return }
    if ($watchExtensions -notcontains $ext) { return }
    
    $script:hasChanges = $true
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] 📝 文件修改: $(Split-Path $path -Leaf)" -ForegroundColor Yellow
} | Out-Null

Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action {
    $path = $Event.SourceEventArgs.FullPath
    $ext = [System.IO.Path]::GetExtension($path)
    
    if (ShouldIgnore $path) { return }
    if ($watchExtensions -notcontains $ext) { return }
    
    $script:hasChanges = $true
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] ➕ 新文件: $(Split-Path $path -Leaf)" -ForegroundColor Yellow
} | Out-Null

Register-ObjectEvent -InputObject $watcher -EventName "Deleted" -Action {
    $path = $Event.SourceEventArgs.FullPath
    if (ShouldIgnore $path) { return }
    
    $script:hasChanges = $true
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] ➖ 文件删除: $(Split-Path $path -Leaf)" -ForegroundColor Yellow
} | Out-Null

Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action {
    $path = $Event.SourceEventArgs.FullPath
    if (ShouldIgnore $path) { return }
    
    $script:hasChanges = $true
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] 🔄 文件重命名: $(Split-Path $path -Leaf)" -ForegroundColor Yellow
} | Out-Null

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Cprog 自动同步工具已启动" -ForegroundColor Cyan
Write-Host "  监控目录: $projectPath" -ForegroundColor Gray
Write-Host "  同步间隔: ${syncInterval}秒" -ForegroundColor Gray
Write-Host "  按 Ctrl+C 停止" -ForegroundColor Gray
Write-Host "========================================" -ForegroundColor Cyan

# 主循环
while ($true) {
    Start-Sleep -Seconds $syncInterval
    
    if ($script:hasChanges -or ((Get-Date) - $script:lastSync).TotalSeconds -ge 300) {
        Sync-ToGitHub
    }
}
