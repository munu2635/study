<?php
	session_start();
?>
<meta charset="utf-8">
<?php
   $hp = $hp1."-".$hp2."-".$hp3;
   $email = $email1."@".$email2;

   $regist_day = date("Y-m-d (H:i)");  // 현재의 '년-월-일-시-분'을 저장

   include "dbconn1.php";       // dconn.php 파일을 불러옴

   $sql = "update memberi set pass='$pass', name='$name' , ";
   $sql .= "email='$email', regist_day='$regist_day' where id='$userid'";

   mysql_query($sql, $connect);  // $sql 에 저장된 명령 실행

   mysql_close();                // DB 연결 끊기
   echo "
	   <script>
	    location.href = 'test1.php';
	   </script>
	";
?>
