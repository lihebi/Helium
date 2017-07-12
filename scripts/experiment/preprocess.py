#!/usr/bin/env python3

# unzip
# preprocess
# generate selection

from threading import Thread
import argparse
import os
import subprocess

class ThreadExecutor(Thread):
    def __init__(self, cmd):
        Thread.__init__(self)
        self.cmd = cmd
    def run(self):
        subprocess.run(self.cmd)

def unzip(zipfolder, unzipfolder):
    """
    unzip all zip files and put into unzip folder with the same name
    """
    if os.path.exists(unzipfolder):
        print('Output folder exists. Remove to continue')
        exit(1)
    for root,dirs,files in os.walk(zipfolder):
        for f in files:
            if f.endswith('.zip'):
                # extract
                tmpfolder=unzipfolder+'/tmp-'+f
                targetfolder = unzipfolder + '/' + f[:-4]
                zipfile = root + '/' + f
                os.makedirs(tmpfolder)
                print ('unzipping ' + zipfile + ' ..')
                subprocess.run('unzip -q ' + zipfile + ' -d ' + tmpfolder, shell=True)
                subprocess.run('mv ' + tmpfolder + '/* ' + targetfolder, shell=True)
                # this folder is empty
                os.rmdir(tmpfolder)

def create_cache(in_folder, out_folder):
    """
    helium preprocess all benchmarks in folder
    """
    for item in os.listdir(in_folder):
        # item will be the folder
        benchmark=os.path.join(in_folder, item)
        outputfolder=os.path.join(out_folder, item)
        if os.path.isdir(benchmark):
            cmd='helium --create-cache ' + benchmark + ' -o ' + outputfolder
            subprocess.run(cmd, shell=True)

def create_selection(in_folder, out_folder):
    """
    Create random selection for all cache folder
    """
    for item in os.listdir(in_folder):
        benchmark=os.path.join(in_folder, item)
        outputfolder=os.path.join(out_folder, item)
        if os.path.isdir(benchmark):
            cmd = 'helium --create-sel --sel-num 100 --sel-tok 10 ' + benchmark + ' -o ' + outputfolder
            subprocess.run(cmd, shell=True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Preprocessing Commands')
    parser.add_argument('--action', choices=['unzip', 'create-cache', 'create-selection'], required=True)
    parser.add_argument('--indir', required=True)
    parser.add_argument('--outdir', required=True)
    args = parser.parse_args();
    if (args.action == 'unzip'):
        unzip(args.indir, args.outdir)
    elif (args.action == 'create-cache'):
        create_cache(args.indir, args.outdir)
    elif (args.action == 'create-selection'):
        create_selection(args.indir, args.outdir)
            
