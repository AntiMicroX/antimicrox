#/bin/bash!

for i in *.cpp; do
 sed -i '/[a-zA-Z]+\s+[a-zA-Z0-9]+::[a-zA-Z0-9]+\((\s*[a-zA-Z0-9]+\s+[a-zA-Z0-9]+,?)*\)\s*\n?\s*{?/a \
qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;' $i
done
