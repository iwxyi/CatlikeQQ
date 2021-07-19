<?php
function deldir($path){
    //如果是目录则递归子目录，继续操作是目录则继续
    if(is_dir($path)){
        //扫描一个文件夹内的所有文件夹和文件并返回数组
        $p = scandir($path);
        //如果 $p 中有两个以上的元素则说明当前 $path 不为空
        if(count($p)>2){
            foreach($p as $val){
                //排除目录中的.和..
                if($val !="." && $val !=".."){
                    //如果是目录则递归子目录，继续操作
                    if(is_dir($path.$val)){
                        //子目录中操作删除文件夹和文件
                        deldir($path.$val.'/');
                    }else{
                        //如果是文件直接删除
                        unlink($path.$val);
                    }
                }
            }
        }
    }
    //删除目录
    return rmdir($path);
}

//设置需要删除的文件夹
$path = "files/";
//调用函数，传入路径
deldir($path);

echo "{'code': 0}";