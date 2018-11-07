#-*- coding:utf-8 –*-
import os
import sys
import glob
import codecs

def get_files(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            yield os.path.join(root, file)

def file_change(file):

    #只修改这几种类型的文件
    if file.endswith('.c'):
#        with open(file,encoding = 'gb2312') as f:
#            with open(file + '_new', "w",encoding = 'gb2312') as f_new:
#                f_new.write(f.read())

#        os.remove(f)
#        os.rename(file + '_new',f)
        str = 

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print("argv[1] can't be empty! enter a directory of file!")
        sys.exit(0)

    #判断传入的文件或者路径是否存在
    elif os.path.exists(sys.argv[1]):
        # 文件
        if os.path.isfile(sys.argv[1]):
            file = sys.argv[1]
            #文件操作
            file_change(file)

        # 路径
        else:
            dir = sys.argv[1]
            for file in get_files(dir):
                #文件操作
                file_change(file)

