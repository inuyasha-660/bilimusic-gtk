# bilimusic-gtk

## 登录
在``音乐源``中点击``登录``打开web登录，登录完成后点击``下一步``，将读取 bilibili 设置的 cookie
``````cookie
DedeUserID
DedeUserID__ckMd5
SESSDATA
bili_jct
sid
``````
cookie 保存于``./bilimusic/account.json``

## 手动导入
### 收藏夹
默认导入全部分p，需确保目标收藏夹公开；数据保存于``./bilimusic/favo/<fid>.json``

### Bvid
可选导入分p(0: 全选)，由Bvid导入的音乐保存于``./bilimusic/music.json``

## 编译
### 依赖库
``gtk4``,``libadwaita-1``, ``libcurl``, ``cJSON``, ``webkitgtk-6.0``

### Meson
``````shell
meson setup build
cd build
meson compile
``````
