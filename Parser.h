#pragma once


#include <regex>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

struct position
{
    size_t row;
    size_t column;

    string const toString()
    {
        size_t tmp = column;
        string str;
        while (tmp)
        {
            str = char(tmp % 10 + 48) + str;
            tmp /= 10;
        }
        str = '.' + str;
        tmp = row;

        while (tmp)
        {
            str = char(tmp % 10 + 48) + str;
            tmp /= 10;
        }

        return str;
    }

    void operator=(size_t arg)
    {
        column = arg;
    }

    bool operator==(position p)
    {
        return (row == p.row) && (column == p.column);
    }

    bool includes(position p)
    {
        if (row == p.row)
        {
            return column <= p.column;
        }
        else
        {
            return row <= p.row;
        }
    }
    position()
    {
        row = 0;
        column = 0;
    }

    bool is_null()
    {
        return row == 0 && column == 0;
    }
};

struct block
{
    string _class;
    string function;
    position block_pos;

    string get_title()
    {
        string str;

        if (_class != "\0") str += _class + "::";
        if(function != "\0") str += function + "()::";
        str += " " + block_pos.toString();

        return str;
    }
    bool includes(block bl)
    {
        if(_class != bl._class) return false;
        if(function == "") return true;
        else
        {
            if(function != bl.function) return false;
            else
            {
                if(block_pos == bl.block_pos) return true;
                else
                {
                    if(block_pos.includes(bl.block_pos)) return true;
                    return false;
                }
            }
        }
    }
};

struct variable
{
    string type;
    string name;
    string def_value;
    position init;

    vector<position> change_pos;
    string toString()
    {
        string str = type + " " + name + " " + def_value + " " + init.toString();
        return str;
    }
};



struct prototype
{
    string _class;
    position pos;
    string type;
    string name;
    size_t reloads;
    vector<variable> parameters;
    prototype():reloads(1){};
};

struct block_info
{
    block place;

    vector<variable> variables;

    int proto;

    block_info(): proto(-1){}
};

struct _if
{
    size_t dif;
    position pos;
};

class Parser
{
public:
    vector<block_info>  blocks_info;
    string code;

    vector<prototype> prototypes;
    size_t reloads;

    vector<position> errors;
    vector<_if> ifs;

    vector<string> personal_types;
    string p_types;
    string types;
    void add_type(string);

    regex func_expr();
    regex variable_expr();
    regex array_expr();
    regex arg_expr();

    regex change_expr[6];//
    regex if_expr;//
    regex while_expr;
    regex for_expr;
    regex logical_errors_expr[2];
    regex class_expr;//

    std::string func_postf;
    std::string variable_postf;
    std::string arg_postf;//
    string array_postf;//

    void hide(string&, regex, size_t);

    void search_classes();	//сделать добавить лог ошибки приколы с прототипами итд + указатели + ссылки
    void search_methods();
    void search_errors();
    void count_if();
    void add_change(string, position, size_t);
    void block_checking(string, block_info, size_t);

    void correct_position();

    void qSort(vector<position*>&, int first, int last);

    void count_reloads();

    size_t rec_func(string inp, size_t pos);
    Parser();
    void set_code(string const& _code);

    void do_parse();

    size_t block_length(size_t);
    size_t arguments_length(size_t);

    void reset();
};
