/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "njson.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "map.h"

namespace crown
{
namespace njson
{
	static const char* next(const char* str, const char c = 0)
	{
		CE_ASSERT_NOT_NULL(str);

		if (c && c != *str)
		{
			CE_ASSERT(false, "Expected '%c' got '%c'", c, *str);
		}

		return ++str;
	}

	static const char* skip_string(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		bool escaped = false;

		while ((*(str = next(str))) != 0)
		{
			if (*str == '"' && !escaped)
			{
				str = next(str);
				return str;
			}
			else if (*str == '\\') escaped = true;
			else escaped = false;
		}

		return str;
	}

	static const char* skip_value(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		switch (*str)
		{
			case '"': str = skip_string(str); break;
			case '[': str = skip_block(str, '[', ']'); break;
			case '{': str = skip_block(str, '{', '}'); break;
			default: for (; *str != ',' && *str != '\n' && *str != ' ' && *str != '}' && *str != ']'; ++str) ; break;
		}

		return str;
	}

	static const char* skip_comments(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		if (*str == '/')
		{
			str = next(str, '/');
			str = next(str, '/');
			while (*str && *str != '\n') str = next(str);
		}

		return str;
	}

	static const char* skip_spaces(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		while (*str)
		{
			if (*str == '/') str = skip_comments(str);
			else if (isspace(*str) || *str == ',') ++str;
			else break;
		}

		return str;
	}

	NJSONType::Enum type(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		switch (*str)
		{
			case '"': return NJSONType::STRING;
			case '{': return NJSONType::OBJECT;
			case '[': return NJSONType::ARRAY;
			case '-': return NJSONType::NUMBER;
			default: return (isdigit(*str)) ? NJSONType::NUMBER : (*str == 'n' ? NJSONType::NIL : NJSONType::BOOL);
		}
	}

	void parse_string(const char* str, DynamicString& string)
	{
		CE_ASSERT_NOT_NULL(str);

		if (*str == '"')
		{
			while (*(str = next(str)))
			{
				// Empty string
				if (*str == '"')
				{
					str = next(str);
					return;
				}
				else if (*str == '\\')
				{
					str = next(str);

					switch (*str)
					{
						case '"': string += '"'; break;
						case '\\': string += '\\'; break;
						case '/': string += '/'; break;
						case 'b': string += '\b'; break;
						case 'f': string += '\f'; break;
						case 'n': string += '\n'; break;
						case 'r': string += '\r'; break;
						case 't': string += '\t'; break;
						default:
						{
							CE_FATAL("Bad escape character");
							break;
						}
					}
				}
				else
				{
					string += *str;
				}
			}
		}

		CE_FATAL("Bad string");
	}

	static const char* parse_key(const char* str, DynamicString& key)
	{
		CE_ASSERT_NOT_NULL(str);

		if (*str == '"')
		{
			parse_string(str, key);
			return skip_string(str);
		}
		else if (isalpha(*str))
		{
			while (true)
			{
				if (isspace(*str) || *str == '=') return str;

				key += *str;
				++str;
			}
		}

		CE_FATAL("Bad key");
	}

	double parse_number(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		TempAllocator512 alloc;
	 	Array<char> number(alloc);

		if (*str == '-')
		{
			array::push_back(number, '-');
			str = next(str, '-');
		}
		while (isdigit(*str))
		{
			array::push_back(number, *str);
			str = next(str);
		}

		if (*str == '.')
		{
			array::push_back(number, '.');
			while ((*(str = next(str))) && isdigit(*str))
			{
				array::push_back(number, *str);
			}
		}

		if (*str == 'e' || *str == 'E')
		{
			array::push_back(number, *str);
			str = next(str);

			if (*str == '-' || *str == '+')
			{
				array::push_back(number, *str);
				str = next(str);
			}
			while (isdigit(*str))
			{
				array::push_back(number, *str);
				str = next(str);
			}
		}

		// Ensure null terminated
		array::push_back(number, '\0');
		return parse_double(array::begin(number));
	}

	bool parse_bool(const char* str)
	{
		CE_ASSERT_NOT_NULL(str);

		switch (*str)
		{
			case 't':
			{
				str = next(str, 't');
				str = next(str, 'r');
				str = next(str, 'u');
				str = next(str, 'e');
				return true;
			}
			case 'f':
			{
				str = next(str, 'f');
				str = next(str, 'a');
				str = next(str, 'l');
				str = next(str, 's');
				str = next(str, 'e');
				return false;
			}
			default:
			{
				CE_FATAL("Bad boolean");
				return false;
			}
		}
	}

	int32_t parse_int(const char* str)
	{
		return (int32_t) parse_number(str);
	}

	float parse_float(const char* str)
	{
		return (float) parse_number(str);
	}

	void parse_array(const char* str, Array<const char*>& array)
	{
		CE_ASSERT_NOT_NULL(str);

		if (*str == '[')
		{
			str = next(str, '[');
			str = skip_spaces(str);

			if (*str == ']')
			{
				str = next(str, ']');
				return;
			}

			while (*str)
			{
				array::push_back(array, str);

				str = skip_value(str);
				str = skip_spaces(str);

				if (*str == ']')
				{
					str = next(str, ']');
					return;
				}

				str = skip_spaces(str);
			}
		}

		CE_FATAL("Bad array");
	}

	void parse_object(const char* str, Map<DynamicString, const char*>& object)
	{
		CE_ASSERT_NOT_NULL(str);

		if (*str == '{')
		{
			str = next(str, '{');

			str = skip_spaces(str);

			if (*str == '}')
			{
				next(str, '}');
				return;
			}

			while (*str)
			{
				DynamicString key;
				str = parse_key(str, key);

				str = skip_spaces(str);
				str = next(str, '=');
				str = skip_spaces(str);

				map::set(object, key, str);

				str = skip_value(str);
				str = skip_spaces(str);

				if (*str == '}')
				{
					next(str, '}');
					return;
				}

				str = skip_spaces(str);
			}
		}

		CE_FATAL("Bad object");
	}
} // namespace njson
} // namespace crown