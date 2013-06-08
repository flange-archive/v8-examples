#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <v8.h>

#define FILE_NAME "01.js"


void die(const char *func, const char *msg)
{
	if ((func == NULL) || (msg == NULL)) {
		printf("Error @ die():\n");
		printf("Either func or msg pointer was NULL\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	printf("Error @ %s()\n", func);
	printf("%s\n", msg);
	fflush(stdout);
	exit(EXIT_FAILURE);
}


v8::Handle<v8::String> read_file(const char *file_name)
{
	size_t res;
	long file_size;
	FILE *file;
	char *buf;
	v8::Handle<v8::String> file_content;

	if (file_name == NULL)
		die("read_file", "Given file name was NULL");

	file = fopen(file_name, "rb");
	if (file == NULL) {
		fclose(file);
		die("read_file", "Couldn't open file");
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);;
	rewind(file);

	buf = new char[file_size + 1];
	buf[file_size] = '\0';

	res = fread(buf, file_size, 1, file);
	if (res != 1) {
		fclose(file);
		die("read_file", "Couldn't read from file");
	}

	file_content = v8::String::New(buf, file_size);

	fclose(file);
	delete[] buf;

	return file_content;
}


v8::Handle<v8::Value> Print(const v8::Arguments &args)
{
	for (int i = 0; i < args.Length(); i++) {
		v8::String::Utf8Value str(args[i]);
		if (*str == NULL)
			break;
		else
			printf("%s ", *str);
	}
	printf("\n");
	fflush(stdout);

	return v8::Undefined();
}


int main(void)
{
	v8::Isolate *isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);

	v8::Handle<v8::String> script_src  = read_file(FILE_NAME);
	v8::Handle<v8::Value> script_name  = v8::String::New(FILE_NAME);

	v8::Handle<v8::ObjectTemplate> global  = v8::ObjectTemplate::New();
	global->Set(v8::String::New("print"), v8::FunctionTemplate::New(Print));

	v8::Handle<v8::Context> context = v8::Context::New(isolate, NULL, global);
	v8::Context::Scope context_scope(context);

	v8::Handle<v8::Script> script;
	script = v8::Script::Compile(script_src, script_name);

	if (script.IsEmpty())
		die("main", "Couldn't compile script");

	script->Run();

	return EXIT_SUCCESS;
}
