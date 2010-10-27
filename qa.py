#!/usr/bin/env python
# encoding: utf-8
# Julien Charbon, 2009

import Logs, Task, Constants, Utils, Options
from TaskGen import before, after, feature

import os, sys, threading

qalock = threading.Lock()

@feature('qa')
@after('apply_link', 'vars_target_cprogram')
def make_qa(self):

    if not getattr(Options.options, 'qa', False):
        return

    if not 'cprogram' in self.features:
        Logs.error('test cannot be executed %s' % self)
        return

    self.default_install_path = None
    tsk = self.create_task('qa')
    tsk.set_inputs(self.link_task.outputs)

    tsk.qa_argv = getattr(self, 'qa_argv', '')
    tsk.qa_cwd = getattr(self, 'qa_cwd', self.link_task.outputs[0].parent.abspath(self.env))

    # Add uselib LIBPATH_{LIB} path
    uselib = getattr(self,'uselib',None)
    if uselib:
        lst = []
        for lib in Utils.to_list(uselib):
            for path in self.env['LIBPATH_' + lib]:
                if not path in lst:
                    lst.append(path)
        tsk.ld_library_path = lst

class ReadStdout(threading.Thread):
    def __init__(self, event, proc):
	    threading.Thread.__init__(self)
            self.event = event
            self.proc = proc
    def run(self):

        while True:
            o = self.proc.stdout.readline()
            if o == '' and self.proc.poll() != None: break
            sys.stdout.write(o)
            sys.stdout.flush()

        self.event.set()

def exec_qa(self):
    qalock.acquire()
    fail = False
    try:
        filename = self.inputs[0].abspath(self.env)
        cmd = filename + ' ' + self.qa_argv
        cwd = self.qa_cwd

        Utils.pprint('CYAN', "Executing qa: '%s' in dir: '%s'" % (cmd, cwd))

        try:
            fu = getattr(self.generator.bld, 'all_test_paths')
        except AttributeError:
            fu = os.environ.copy()
            self.generator.bld.all_test_paths = fu

            lst = getattr(self, 'ld_library_path', [])
            for obj in self.generator.bld.all_task_gen:
                link_task = getattr(obj, 'link_task', None)
                if link_task:
                    lst.append(link_task.outputs[0].parent.abspath(obj.env))

            def add_path(dct, path, var):
                dct[var] = os.pathsep.join(Utils.to_list(path) + [os.environ.get(var, '')])
            if sys.platform == 'win32':
                add_path(fu, lst, 'PATH')
            elif sys.platform == 'darwin':
                add_path(fu, lst, 'DYLD_LIBRARY_PATH')
                add_path(fu, lst, 'LD_LIBRARY_PATH')
            else:
                add_path(fu, lst, 'LD_LIBRARY_PATH')

        try:
            import pproc
            p = pproc.Popen(cmd, cwd=cwd, env=fu, shell=True, stdout=pproc.PIPE, stderr=pproc.STDOUT)

            event = threading.Event()
            read = ReadStdout(event, p)
            read.start()
            event.wait()

            ret = ''
	except OSError, e:
            fail = True
            ret = '' + str(e)
        else:
            if p.returncode:
                fail = True

        ret = ret + 'returncode: %d' % p.returncode
        stats = getattr(self.generator.bld, 'qa_results', [])
        stats.append((cmd, fail, ret))
        self.generator.bld.qa_results = stats
    finally:
        qalock.release()

cls = Task.task_type_from_func('qa', func=exec_qa, color='RED')

# Run every time
cls.runnable_status = lambda self: Constants.RUN_ME

def summary(bld):
    lst = getattr(bld, 'qa_results', [])
    if lst:
        Utils.pprint('CYAN', 'Execution summary')
        for (f, fail, ret) in lst:
            col = fail and 'RED' or 'GREEN'
            Utils.pprint(col, (fail and 'FAIL' or 'ok') + " " + f)
            if fail: Utils.pprint('RED', ret.replace('\\n', '\n'))
            else: Utils.pprint('CYAN', ret.replace('\\n', '\n'))
