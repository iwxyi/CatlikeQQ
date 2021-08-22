<?php
// 确保文件存在
if (strlen($_FILES["upfile"]["name"]) < 1) {
    die('文件不能为空');
}
// 确保目录存在
$dir = "files";
if(!is_dir($dir)){
    mkdir(iconv("UTF-8", "GBK", $dir), 0777, true);
}

// 哈希并放入文件
$hash = md5_file($_FILES['upfile']['tmp_name']);
$result = move_uploaded_file($_FILES["upfile"]["tmp_name"], "files/$hash");

echo "{'hash': '$hash'}";

// 清理过期文件
function del_file_by_time($dir,$n)
{
    if(is_dir($dir)){
        if($dh=opendir($dir)){
            while (false !== ($file = readdir($dh))){
                if($file!="." && $file!=".."){
                    $fullpath=$dir."/".$file;
                    if(!is_dir($fullpath)){
                        $filedate=filemtime($fullpath);
                        $minutes=round((time()-$filedate)/60);
                        if($minutes>$n)
                            unlink($fullpath); //删除文件
                    }
                }
            }
        }
        closedir($dh);
    }
}

// 清理一小时前的文件
del_file_by_time('files', 60);