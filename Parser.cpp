#include "Parser.h"
#define info blocks_info[index]


Parser::Parser()
{
try
{
    array_postf = "\\s*(\\**)\\s*(\\w+)\\s*(\\[\\w*\\])(\\s*=\\s*([^;]*))?\\s*;";
    func_postf = "(\\w+)\\s*\\([^\\{]*\\)\\s*\\{";
    variable_postf = "\\s*([\\*&]*)\\s*(\\w+)\\s*(=\\s*(\\w+))?\\s*;";
    arg_postf = "\\s*(\\**)\\s*(\\w+)\\s*(\\[\\w*\\])?";		//параметры функции
    class_expr = "(class|struct)\\s+(\\w+)\\s*";

    change_expr[0] = "(\\w+)\\s*(\\[\\.*\\])?\\s*=[^;]*;";
    change_expr[1] = "\\+\\+(\\w+)\\s*(\\[.*\\])?";
    change_expr[2] = "--(\\w+)\\s*(\\[.*\\])?";
    change_expr[3] = "(\\w+)\\s*(\\[.*\\])?\\+\\+";
    change_expr[4] = "(\\w+)\\s*(\\[.*\\])?--";
    change_expr[5] = "(\\w+)\\s*(\\[.*\\])?\\s*[\\+-/\\*]=[^;];";

    for_expr = "\\bfor[\\b\\s\\(]";
    if_expr = "\\b(?:if[\\b\\s\\(]|else[\\b\\s])";
    while_expr = "\\bwhile[\\b\\s\\(]";

    types = "(int|char|float|double|bool|long|long long|short|size_t|void|long double|string|regex|vector<[^>]+>)";	//это стринг
    p_types = "()";
}
catch (regex_error& err)
{
    cout << err.what() << "\n";
    if (err.code() == std::regex_constants::error_brack) {
        std::cout << "The code was error_brack\n";
    }
}
}

void Parser::add_type(string type)
{
    personal_types.push_back(type);
    size_t length = types.length();
    types[length - 1] = '|';
    types += type;
    types += ')';

    length = p_types.length();
    if (length == 2)
    {
        p_types = '(' + type + ')';
    }
    else
    {
        p_types[length - 1] = '|';
        p_types += personal_types.back();
        p_types += ')';
    }
}

regex Parser::func_expr()
{
    string str = types;
    str +="(\\**)\\s*(";
    str += p_types;
    str += "::)?\\s*";
    str += func_postf;
    return regex(str);
}

regex Parser::variable_expr()
{
    string str = types;
    str += variable_postf;
    return regex(str);
}

regex Parser::array_expr()
{
    string str = types;
    str += array_postf;
    return regex(str);
}

regex Parser::arg_expr()
{
try{
    string str = types;
    str += arg_postf;
    return regex(str);
    }
catch (regex_error& err)
{
    cout << err.what() << "\n";
    if (err.code() == std::regex_constants::error_brack) {
        std::cout << "The code was error_brack\n";
    }
    exit(5);
}
}


void Parser::search_classes()
{

    string inp = code;
    smatch sm;

    size_t pos = 0;
    while (regex_search(inp, sm, class_expr))
    {
        add_type(sm[2].str());

        block_info blck;
        blck.place._class = sm[2].str();

        size_t length = block_length(pos + sm.position(0));
        block_checking(inp.substr(sm.position(0), length), blck, pos + sm.position(0));

        inp = inp.substr(sm.position(0) + length);
        pos += sm.position(0) + length;
    }
}

void Parser::block_checking(string blck, block_info in, size_t pos)
{
    smatch sm;
    size_t index = blocks_info.size();
    string inp = blck;
    size_t current_pos = pos;

    blocks_info.push_back(in);
    //auto info = blocks_info.end();

    hide(inp, for_expr, pos);
    hide(inp, while_expr, pos);
    hide(inp, if_expr, pos);

    string inp_tmp = inp;
    while (regex_search(inp_tmp, sm, variable_expr()))
    {
        variable v;
        v.type = sm[1].str() + sm[2].str();
        v.name = sm[3];
        v.init = current_pos + sm.position(0);
        v.def_value = sm[4];

        info.variables.push_back(v);

        inp_tmp = sm.suffix().str();
        current_pos += sm.position(0) + sm.length(0);
    }

    current_pos = pos;
    inp_tmp = inp;
    while (regex_search(inp_tmp, sm, array_expr()))
    {
        variable v;
        v.type = sm[1].str() + sm[2].str() + sm[4].str();
        v.name = sm[3];
        v.init = current_pos + sm.position(0);
        v.def_value = sm[5];

        info.variables.push_back(v);

        inp_tmp = sm.suffix().str();
        current_pos += sm.position(0) + sm.length(0);
    }

    for (size_t i = 0; i < 6; ++i)
    {
        current_pos = pos;
        inp_tmp = inp;
        while (regex_search(inp_tmp, sm, change_expr[i]))
        {
            position p;
            p = current_pos + sm.position(0);
            add_change(sm[1], p, index);

            inp_tmp = sm.suffix().str();
            current_pos += sm.position(0) + sm.length(0);
        }
    }

    inp_tmp = blck;
    current_pos = pos;
    while (regex_search(inp_tmp, sm, for_expr))
    {
        size_t length = block_length(current_pos + sm.position(0));
        block_info bl;
        bl.place = info.place;
        bl.place.block_pos = current_pos + sm.position(0);

        block_checking(inp_tmp.substr(sm.position(0) + 3, length - 3), bl,
                        current_pos + sm.position(0) + 3);

        inp_tmp = inp_tmp.substr(sm.position(0) + length - 1);
        current_pos += sm.position(0) + length - 1;
    }

    inp_tmp = blck;
    current_pos = pos;
    while (regex_search(inp_tmp, sm, while_expr))
    {
        size_t length = block_length(current_pos + sm.position(0));
        block_info bl;
        bl.place = info.place;
        bl.place.block_pos = current_pos + sm.position(0);

        block_checking(inp_tmp.substr(sm.position(0) + 5, length - 5), bl,
            current_pos + sm.position(0) + 5);

        inp_tmp = inp_tmp.substr(sm.position(0) + length - 1);
        current_pos += sm.position(0) + length - 1;
    }

    inp_tmp = blck;
    current_pos = pos;
    while (regex_search(inp_tmp, sm, if_expr))
    {
        size_t length = block_length(current_pos + sm.position(0));
        block_info bl;
        bl.place = info.place;

        bl.place.block_pos = current_pos + sm.position(0);


        block_checking(inp_tmp.substr(sm.position(0) + sm.length(0), length - sm.length(0)), bl,
            current_pos + sm.position(0) + sm.length(0));

        inp_tmp = inp_tmp.substr(sm.position(0) + length - 1);
        current_pos += sm.position(0) + length - 1;
    }

}

void Parser::search_methods()
{
    string inp = code;
    size_t current_pos = 0;
    smatch sm;

    while (regex_search(inp, sm, func_expr()))
    {
        prototype proto;
        proto.type = sm[1].str() + sm[2].str();
        proto.name = sm[5];
        proto._class = sm[4];
        proto.pos = current_pos + sm.position(0);

        block_info inf;
        inf.place.function = proto.name;
        inf.place._class = proto._class;
        inf.place.block_pos = proto.pos;

        size_t param_length = arguments_length(current_pos + sm.position(5) + sm.length(5));
        string inp_tmp = inp.substr(sm.position(5) + sm.length(5), param_length);
        smatch sm_tmp;

        size_t curr_pos_tmp = current_pos + sm.position(5) + sm.length(5);
        while (regex_search(inp_tmp, sm_tmp, arg_expr()))
        {
            variable v;
            v.type = sm_tmp[1].str() + sm_tmp[2].str() + sm_tmp[4].str();
            v.name = sm_tmp[3];
            v.init = curr_pos_tmp + sm_tmp.position(0);
            v.def_value = sm_tmp[5];
            inf.variables.push_back(v);

            inp_tmp = sm_tmp.suffix();
            curr_pos_tmp += sm_tmp.position(0) + sm_tmp.length(0);
        }

        size_t length = block_length(current_pos + sm.position(0));
        string txt = inp.substr(sm.position(0), length);
        prototypes.push_back(proto);
        inf.proto = prototypes.size() - 1;
        block_checking(txt, inf, current_pos + sm.position(0));

        inp = inp.substr(sm.position(0) + length - 1);
        current_pos += sm.position(0) + length - 1;
    }
}

void Parser::add_change(string name, position pos, size_t index_of_current_block)
{
    block_info current_block = blocks_info[index_of_current_block];

    for (int index = blocks_info.size() - 1; index >= 0; --index)
    {
        block_info &rf = blocks_info[index];
        if (rf.place.includes(current_block.place))
        {
            for (size_t t = 0; t < rf.variables.size(); ++t)
            {
                if (rf.variables[t].name == name)
                {
                    rf.variables[t].change_pos.push_back(pos);
                    return;
                }
            }
        }
    }
}

void Parser::hide(string& inp, regex rx, size_t pos)
{
    smatch sm;
    while (regex_search(inp, sm, rx))
    {
        string rplace;
        size_t length = block_length(pos + sm.position(0));
        for(size_t i = 0; i < length; ++i)
            rplace += ' ';

        inp = sm.prefix().str() + rplace + inp.substr(sm.position(0) + length - 1);
    }
}

void Parser::search_errors()
{
    regex rgx("(while|if)\\s*\\((true|false)\\)");
    string inp = code;
    smatch sm;
    size_t pos = 0;
    while (regex_search(inp, sm, rgx))
    {
        position p ;
        p = pos + sm.position(0);
        errors.push_back(p);
        pos += sm.prefix().length() + sm[0].length();
        inp = sm.suffix();
    }
}

void Parser::correct_position()		// в переменных(инит и вектор изменений)  в еррорсах
{
    vector<position*> pos_s;

    for (size_t t = 0; t < blocks_info.size(); ++t)
    {
        block_info &blck = blocks_info[t];

        for (size_t k = 0; k < blck.variables.size(); ++k)
        {
            variable &var = blck.variables[k];
            pos_s.push_back(&(var.init));

            for (size_t h = 0; h < var.change_pos.size(); ++h)
            {
                pos_s.push_back(&(var.change_pos[h]));
            }
        }

        pos_s.push_back(&(blck.place.block_pos));
    }

    for (size_t t = 0; t < prototypes.size(); ++t)
    {
        prototype &pr = prototypes[t];
        pos_s.push_back(&(pr.pos));

        for (size_t k = 0; k < pr.parameters.size(); ++k)
            pos_s.push_back(&(pr.parameters[k].init));

    }

    for (size_t t = 0; t < errors.size(); ++t)
    {
        pos_s.push_back(&(errors[t]));
    }

    for(size_t t = 0; t < ifs.size(); ++t) pos_s.push_back(&(ifs[t].pos));

    qSort(pos_s,0, pos_s.size()-1);

    size_t k = 0;
    while(k < pos_s.size() && pos_s[k]->is_null()) ++k;


    size_t row = 1, column = 0;
    for (size_t t = 0; t < code.length() && k < pos_s.size(); ++t)
    {
        if (t == pos_s[k]->column)
        {
            pos_s[k]->column = column;
            pos_s[k]->row = row;
            ++k;
            --t;
            continue;
        }

        if(code[t] == '\n')
        {
            ++row;
            column = 0;
        }

        ++column;
    }
}

void Parser::qSort(vector<position*> &mas, int first, int last)
{
    position* mid;
    position* tmp;
    int f = first, l = last;
    mid = mas[(f + l) / 2]; //вычисление опорного элемента
    do
    {
        while (mas[f]->column < mid->column) f++;
        while (mas[l]->column > mid->column) l--;
        if (f <= l) //перестановка элементов
        {
            tmp = mas[f];
            mas[f] = mas[l];
            mas[l] = tmp;
            f++;
            l--;
        }
    } while (f < l);
    if (first < l) qSort(mas, first, l);
    if (f < last) qSort(mas, f, last);
}

void Parser::count_reloads()
{
    vector<prototype*> copy;
    for(size_t t = 0; t < prototypes.size(); ++t)
        copy.push_back(&(prototypes[t]));

    while (copy.size() != 0)
    {
        vector<size_t> i_s;
        i_s.push_back(0);
        for (size_t t = 1; t < copy.size(); ++t)
        {
            if(copy[0]->_class == copy[t]->_class && copy[0]->name == copy[t]->name) i_s.push_back(t);
        }

        for (size_t t = 0; t < i_s.size(); ++t)
        {
            copy[i_s[t]]->reloads = i_s.size();
        }

        for (size_t t = 0; t < i_s.size(); ++t)
        {
            copy.erase(copy.begin() + i_s[t] - t);
        }
    }
}

void Parser::count_if()
{
    smatch sm;
    string inp = code;
    size_t current_pos = 0;

    while (regex_search(inp, sm, regex("(if|else)")))
    {
        size_t length = block_length(current_pos + sm.position(0) + sm.length(0));
        size_t dif = rec_func(inp.substr(sm.position(0) + sm.length(0), length), current_pos + sm.position(0) + sm.length(0));

        _if inf;
        inf.pos = current_pos + sm.position(0);
        inf.dif = dif;
        ifs.push_back(inf);

        current_pos += sm.position(0) + sm.length(0);
        inp = inp.substr(sm.position(0) + sm.length(0));
    }
}

size_t Parser::rec_func(string inp, size_t pos)
{
    size_t if_ = 0;
    smatch sm;
    size_t current_pos = pos;

    while (regex_search(inp, sm, regex("(if|else)")))
    {
        size_t length = block_length(current_pos + sm.position(0) + sm.length(0));
        if_ = max(rec_func(inp.substr(sm.position(0) + sm.length(0), length), pos + sm.position(0) + sm.length(0)), if_);
        current_pos += sm.position(0) + sm.length(0) + length;

        inp = inp.substr(sm.position(0) + sm.length(0) + length);
    }
    return if_ + 1;
}

void Parser::do_parse()
{
    search_classes();
    search_methods();
    search_errors();
    count_if();
    count_reloads();
    correct_position();
}

size_t Parser::block_length(size_t start_pos)
{
    size_t bracket_count = 0;
    size_t pos = start_pos;
    for (; pos < code.length() && bracket_count == 0; ++pos)
    {
        if (code[pos] == '(')
        {
            size_t count = 1;
            ++pos;
            while (pos < code.length() && count)
            {
                if(code[pos] == '(') ++count;
                if(code[pos] == ')') --count;
                ++pos;
            }
        }

        if(code[pos] == '{') ++bracket_count;
        if(code[pos] == ';') break;
    }

    if (bracket_count)
    {
        for (; pos < code.length() && bracket_count; ++pos)
        {
            if(code[pos] == '{') ++bracket_count;
            if(code[pos] == '}') --bracket_count;
        }
    }
    else
    {
        for(;pos < code.length(); ++pos) if(code[pos] == ';') break;
    }

    return pos - start_pos + 1;
}

size_t Parser::arguments_length(size_t first_pos)
{
    size_t bracket_count = 0;
    size_t pos = first_pos;
    while (pos < code.length() && !bracket_count)
    {
        if(code[pos] == '(') ++bracket_count;
        ++pos;
    }

    do
    {
        if (code[pos] == '(') ++bracket_count;
        if (code[pos] == ')') --bracket_count;
        ++pos;
    }
    while(pos < code.length() && bracket_count);

    return pos - first_pos + 1;
}

void Parser::set_code(string const& _code)
{
    code = _code;
}

void Parser::reset()
{
    /*    vector<block_info>  blocks_info;
    string code;

    vector<prototype> prototypes;

    vector<position> errors;
    vector<_if> ifs;

    vector<string> personal_types;
    string p_types;
    string types;*/

    blocks_info.clear();
    prototypes.clear();
    errors.clear();
    ifs.clear();
    types = "(int|char|float|double|bool|long|long long|short|size_t|void|long double|string|regex|vector<[^>]+>)";
    p_types = "()";
}
