## bilimusic-gtk

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

## 导入收藏夹
手动导入需要确保目标收藏夹是公开的

所有收藏夹将保存于``./bilimusic/favo/<fid>.json``

## 编译
### 依赖库
``gtk4``,``libadwaita-1``, ``libcurl``, ``cJSON``, ``webkitgtk-6.0``

### Meson
``````shell
meson setup build
cd build
meson compile
``````
