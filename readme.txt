・コンパイル方法
<サーバ側>
gcc Server.c DieWithError.c -o Server

<クライアント側>
gcc Client.c DieWithError.c -o Client


・実行方法
<サーバ側>
./Server 5000 10 2
第1引数 ポート番号 
第2引数 行列のサイズ size
第3引数 スレッド数(1,2,4など) thread_count
※sizeが必ずthread_countで割り切れること。
※スレッド数は10未満であること。

<クライアント側>
./Client 127.0.0.1 5000 10 matrixA.txt matrixB.txt matrixC.txt 
第1引数 サーバのIPアドレス
第2引数 ポート番号
第3引数 行列のサイズ size
第4引数 行列Aの入力データのファイル名
第5引数 行列Bの入力データのファイル名
第6引数 行列Cの出力データのファイル名(option)
※行列A, Bの各成分は必ず0-9の整数であること。