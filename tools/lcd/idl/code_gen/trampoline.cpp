#include "ccst.h"
#include "code_gen.h"

/*
 * Code related to function pointers and the trampoline code generated for them.
 * 
 */


/*
 * trampoline function declaration.
 */
CCSTDeclaration* trampoline_function_declaration(Rpc* r)
{
  /*
  LCD_TRAMPOLINE_DATA(new_file_trampoline);
  int 
  LCD_TRAMPOLINE_LINKAGE(new_file_trampoline)
  new_file_trampoline(int id, struct file **file_out)
  */
  std::vector<CCSTDecSpecifier*>specifier = type2(r->return_variable()->type());
  
  printf("done calling type2 tramp func dec\n");
  std::vector<CCSTInitDeclarator*> decs;

  std::vector<CCSTParamDeclaration*> tramp_func_params;
  
  std::vector<Parameter*> parameters = r->parameters();

  decs.push_back(new CCSTDeclarator(pointer(r->return_variable()->pointer_count())
				    , new CCSTDirectDecParamTypeList(new CCSTDirectDecId(trampoline_func_name(r->name()))
								     , parameter_list(parameters))));
  printf("finishing tramp func dec\n");
  return new CCSTDeclaration(specifier, decs);
}

/*
 * generates the body of an rpc which is a function pointer
 * although this function does not verify that.
 * 1. declares a new volatile function pointer
 * 2. declares an instance of the relevant function pointers hidden args structure
 * 3. calls LCD_TRAMPOLINE_LINKAGE macro
 * 4. sets function pointer from step (1) equal to glue code for function pointer
 * 5. returns call to function pointer in step (1)
 */
CCSTCompoundStatement* trampoline_function_body(Rpc* r)
{
  std::vector<CCSTDeclaration*> declarations;
  std::vector<CCSTStatement*> statements;
  
  /* start new function pointer declaration */

  std::vector<CCSTDecSpecifier*> new_fp_return_type = type2(r->return_variable()->type());

  /* loop through rpc parameters and add them to the parameters for the new fp*/
  std::vector<CCSTParamDeclaration*> func_pointer_params;

  std::vector<Parameter*> parameters = r->parameters();
  for(std::vector<Parameter*>::iterator it = parameters.begin(); it != parameters.end(); it ++) {
    Parameter *p = *it;
    
    std::vector<CCSTDecSpecifier*> fp_param_tmp = type2(p->type());
    func_pointer_params.push_back(new CCSTParamDeclaration(fp_param_tmp
							   , new CCSTDeclarator(pointer(p->pointer_count()), new CCSTDirectDecId(""))));
  }

  // add hidden args to function pointer arg list
  std::vector<Parameter*> hidden_args = r->hidden_args_;
  for(std::vector<Parameter*>::iterator it = hidden_args.begin(); it != hidden_args.end(); it ++) {
    Parameter *p = *it;
    
    std::vector<CCSTDecSpecifier*> fp_param_tmp = type2(p->type());
    func_pointer_params.push_back(new CCSTParamDeclaration(fp_param_tmp
							   , new CCSTDeclarator(pointer(p->pointer_count()), new CCSTDirectDecId(""))));
  }

  /* declare new function pointer */
  std::vector<type_qualifier> pointer_type_qualifier;
  pointer_type_qualifier.push_back(volatile_t);

  std::vector<CCSTInitDeclarator*> new_fp_declaration;
  new_fp_declaration.push_back(new CCSTDeclarator(0x0
						  , new CCSTDirectDecParamTypeList(new CCSTDirectDecDec(new CCSTDeclarator(new CCSTPointer(pointer_type_qualifier)
															   , new CCSTDirectDecId("new_fp"))) // todo
										   , new CCSTParamList(func_pointer_params))));

  declarations.push_back(new CCSTDeclaration(new_fp_return_type, new_fp_declaration));
  
  /* end new_fp declaration */

  /* declare a hidden args instance */
  declarations.push_back(struct_pointer_declaration(hidden_args_name(r->name()), "hidden_args", r->current_scope()));

  // LCD_TRAMPOLINE_PROLOGUE

  std::vector<CCSTAssignExpr*> lcd_tramp_prolog_args;
  lcd_tramp_prolog_args.push_back(new CCSTPrimaryExprId("hidden_args"));
  lcd_tramp_prolog_args.push_back(new CCSTPrimaryExprId(trampoline_func_name(r->name())));
  // LCD_TRAMPOLINE_PROLOGUE(hidden_args, new_file_trampoline);
  statements.push_back(new CCSTExprStatement( function_call("LCD_TRAMPOLINE_PROLOGUE", lcd_tramp_prolog_args)));

  // set new function pointer equal to glue code for function pointer
  // new_filep = new_file;
  statements.push_back(new CCSTExprStatement( new CCSTAssignExpr(new CCSTPrimaryExprId("new_fp"), equals(), new CCSTPrimaryExprId(r->name()))));

  // return call from new function pointer
  std::vector<CCSTAssignExpr*> new_fp_args;

 
  for(std::vector<Parameter*>::iterator it = parameters.begin(); it != parameters.end(); it ++) {
    Parameter *p = *it;
    new_fp_args.push_back(new CCSTPrimaryExprId(p->identifier()));
  }

  int err;
  ProjectionType *hidden_args_param = dynamic_cast<ProjectionType*>(r->current_scope()->lookup(hidden_args_name(r->name()), &err));

  // create a tmp variable to do access.
  Parameter *tmp_hidden_args = new Parameter(hidden_args_param, "hidden_args", 1);

  Assert(hidden_args_param != 0x0, "Error: dynamic cast to Projection type failed!\n");

  ProjectionField *cspace_field = hidden_args_param->get_field("cspace");
  if(cspace_field != 0x0) {
    Variable *save_accessor = cspace_field->accessor();
    cspace_field->set_accessor(tmp_hidden_args);
    new_fp_args.push_back(access(cspace_field));
    cspace_field->set_accessor(save_accessor);
  }

  ProjectionField *container_field = hidden_args_param->get_field("container");
  if(container_field != 0x0) {
    Variable *save_accessor = container_field->accessor();
    container_field->set_accessor(tmp_hidden_args);
    new_fp_args.push_back(access(container_field));
    container_field->set_accessor(save_accessor);
  }

  

  statements.push_back(new CCSTReturn(function_call("new_fp", new_fp_args)));

  return new CCSTCompoundStatement(declarations, statements);
}

/* this function is grossly complex. try not to touch it.*/
CCSTCompoundStatement* alloc_init_hidden_args_struct(ProjectionType *pt, Variable *v, LexicalScope *ls, Variable *cspace) 
{
  std::vector<CCSTDeclaration*> declarations;
  std::vector<CCSTStatement*> statements;

  std::vector<ProjectionField*> fields = pt->fields();
  for(std::vector<ProjectionField*>::iterator it = fields.begin(); it != fields.end(); it ++) {
    ProjectionField *pf = *it;
    if(pf->type()->num() == 7 && pf->alloc_callee()) {
      // allocate hidden args structure 
      statements.push_back(kzalloc_structure(hidden_args_name(pf->type()->name())
					     ,hidden_args_name(append_strings("_"
									      , construct_list_vars(pf)))));
      // error check
      statements.push_back(if_cond_fail(new CCSTUnaryExprCastExpr( Not()
								   , new CCSTPrimaryExprId(hidden_args_name(append_strings("_"
															   , construct_list_vars(pf)))))
					, "kzalloc hidden args"));

      // duplicate the trampoline
      // 1. get t_handle field from our hidden args field
      int err;
      Type *hidden_args_type = ls->lookup(hidden_args_name(pf->type()->name()), &err);
      Assert(hidden_args_type != 0x0, "Error: could not find a hidden args type in scope\n");
      ProjectionType *hidden_args_structure = dynamic_cast<ProjectionType*>(hidden_args_type);
      Assert(hidden_args_structure != 0x0, "Error: dynamic cast to projection type failed\n");
      
      ProjectionField *t_handle_field = hidden_args_structure->get_field("t_handle");
      Assert(t_handle_field != 0x0, "Error: could not find t_handle field in projection\n");

      Parameter *tmp_hidden_args_param = new Parameter(hidden_args_structure
						       , hidden_args_name(append_strings("_"
											 , construct_list_vars(pf))), 1);
      
      Variable *accessor_save = t_handle_field->accessor();
      t_handle_field->set_accessor(tmp_hidden_args_param);

      //
      std::vector<CCSTAssignExpr*> lcd_dup_args;
      lcd_dup_args.push_back(new CCSTPrimaryExprId(trampoline_func_name(pf->type()->name()))); // trampoline function name
      statements.push_back(new CCSTExprStatement( new CCSTAssignExpr(access(t_handle_field)
								     , equals()
								     , function_call("LCD_DUP_TRAMPOLINE"
										     , lcd_dup_args))));
						       
      // error check the duplication
      statements.push_back(if_cond_fail(new CCSTUnaryExprCastExpr( Not()
								   , access(t_handle_field))
					, "duplicate trampoline"));

      // store hidden args in trampoline aka t handle
      ProjectionType *t_handle_structure = dynamic_cast<ProjectionType*>(t_handle_field->type());
      Assert(t_handle_structure != 0x0, "Error: dynamic cast to projection type failed\n");

      ProjectionField *t_handle_hidden_args_field = t_handle_structure->get_field("hidden_args");
      Assert(t_handle_hidden_args_field != 0x0, "Error: could not find hidden_args field in t handle structure\n");

      statements.push_back(new CCSTExprStatement( new CCSTAssignExpr( access(t_handle_hidden_args_field)
								      , equals()
								      , new CCSTPrimaryExprId(hidden_args_name(append_strings("_"
															      , construct_list_vars(pf)))))));

      // store container in hidden args
      ProjectionField *hidden_args_container_field = hidden_args_structure->get_field("container");
      Assert(hidden_args_container_field != 0x0, "Error: could not find container field in hidden args structure\n");

      hidden_args_container_field->set_accessor(tmp_hidden_args_param);

      statements.push_back(new CCSTExprStatement(new CCSTAssignExpr( access(hidden_args_container_field)
								     , equals()
								     , access(v->container()))));

      // store data store in hidden args
      ProjectionField *hidden_args_cspace_field = hidden_args_structure->get_field("cspace");
      Assert(hidden_args_cspace_field != 0x0, "Error: could not find cspace field in hidden args structure\n");

      hidden_args_cspace_field->set_accessor(tmp_hidden_args_param);
      statements.push_back(new CCSTExprStatement(new CCSTAssignExpr( access(hidden_args_cspace_field)
								     , equals()
								     , access(cspace))));

      // put trampoline in the container. last step
      ProjectionType *v_container_type = dynamic_cast<ProjectionType*>(v->container()->type());
      Assert(v_container_type != 0x0, "Error: dynamic cast to projection type failed\n");

      ProjectionField *v_container_real_field = v_container_type->get_field(v->type()->name());
      Assert(v_container_real_field != 0x0, "Error: could not find field in projection\n");
      

      ProjectionType *v_container_real_field_type = dynamic_cast<ProjectionType*>(v_container_real_field->type());
      Assert(v_container_real_field_type != 0x0, "Error: dynamic cast to projection type failed\n");

      ProjectionField *v_container_real_field_func_pointer = v_container_real_field_type->get_field(pf->identifier());
      Assert(v_container_real_field_func_pointer != 0x0, "Error: could not find field in structure\n");

      std::vector<CCSTAssignExpr*> handle_to_tramp_args;
      handle_to_tramp_args.push_back(access(t_handle_field));
      statements.push_back(new CCSTExprStatement(new CCSTAssignExpr( access(v_container_real_field_func_pointer)
								     , equals()
								     , function_call("LCD_HANDLE_TO_TRAMPOLINE"
										     , handle_to_tramp_args))));

      t_handle_field->set_accessor(accessor_save);
      // end of this if statement.
    } else if (pf->type()->num() == 4 || pf->type()->num() == 9) {
      // recurse
      ProjectionType *pt_tmp = dynamic_cast<ProjectionType*>(pf->type());
      statements.push_back(alloc_init_hidden_args_struct(pt_tmp, pf, ls, cspace));
    }
  }

  return new CCSTCompoundStatement(declarations, statements);
}

std::vector<CCSTDeclaration*> declare_hidden_args_structures(ProjectionType *pt, LexicalScope *ls)
{
  std::vector<CCSTDeclaration*> declarations;
  
  std::vector<ProjectionField*> fields = pt->fields();
  for(std::vector<ProjectionField*>::iterator it = fields.begin(); it != fields.end(); it ++) {
    ProjectionField *pf = *it;
    
    if(pf->type()->num() == 7) { // function
      // declare
      const char* hidden_args_struct_name = hidden_args_name(pf->type()->name());
      const char* var_name = hidden_args_name(append_strings("_"
							     , construct_list_vars(pf)));

      declarations.push_back(struct_pointer_declaration(hidden_args_struct_name
							, var_name
							, ls));

    } else if (pf->type()->num() == 4 || pf->type()->num() == 9) {
      // recrurse
      ProjectionType *pf_pt = dynamic_cast<ProjectionType*>(pf->type());
      Assert(pf_pt != 0x0, "Error: dynamic cast to projection type failed\n");
      
      std::vector<CCSTDeclaration*> tmp_decs = declare_hidden_args_structures(pf_pt, ls);
      declarations.insert(declarations.end(), tmp_decs.begin(), tmp_decs.end());
    }
  }
  
  return declarations;
}
