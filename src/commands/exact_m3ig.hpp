/* also: Advanced Logic Synthesis and Optimization tool
 * Copyright (C) 2019- Ningbo University, Ningbo, China */

/**
 * @file exact_m3ig.hpp
 *
 * @brief exact synthesis using MIG as the underlying data
 * structure
 *
 * @author Zhufei Chu
 * @since  0.1
 */

#ifndef EXACT_M3IG_HPP
#define EXACT_M3IG_HPP

#include <alice/alice.hpp>
#include <mockturtle/mockturtle.hpp>

#include "../store.hpp"
#include "../core/exact_m3ig_encoder.hpp"

namespace alice
{

  class exact_m3ig_command: public command
  {
    public:
      explicit exact_m3ig_command( const environment::ptr& env ) : 
                      command( env, "using exact synthesis to find optimal M3IGs" )
      {
        add_flag( "--verbose, -v",   "print the information" );
        add_flag( "--cegar, -c",     "CEGAR encoding" );
        add_flag( "--enumerate, -e", "enumerate all the solutions" );
      }

      rules validity_rules() const
      {
        return { has_store_element<optimum_network>( env ) };
      }

      private:
      std::string print_expr( const also::mig3& mig3, const int& step_idx )
      {
        std::stringstream ss;
        std::vector<char> inputs;

        for( auto i = 0; i < 3; i++ )
        {
          if( mig3.steps[step_idx][i] == 0 )
          {
            inputs.push_back( '0' );
          }
          else
          {
            inputs.push_back( char( 'a' + mig3.steps[step_idx][i] - 1 ) );
          }
        }

        switch( mig3.operators[ step_idx ] )
        {
          default:
            break;

          case 0:
            ss << "<" << inputs[0] << inputs[1] << inputs[2] << "> "; 
            break;
          
          case 1:
            ss << "<!" << inputs[0] << inputs[1] << inputs[2] << "> "; 
            break;

          case 2:
            ss << "<" << inputs[0] << "!" << inputs[1] << inputs[2] << "> "; 
            break;

          case 3:
            ss << "<" << inputs[0] << inputs[1] << "!" << inputs[2] << "> "; 
            break;
        }

        return ss.str();
      }

      std::string print_all_expr( const spec& spec, const also::mig3& mig3 )
      {
        std::stringstream ss;

        char pol = spec.out_inv ? '!' : ' ';

        std::cout << "[i] " << spec.nr_steps << " steps are required " << std::endl;
        for(auto i = 0; i < spec.nr_steps; i++ )
        {
          if(  i == spec.nr_steps - 1 ) 
          {
            ss << pol;
            ss << char( i + spec.nr_in + 'a' ) << "=" << print_expr( mig3, i ); 
          }
          else
          {
            ss << char( i + spec.nr_in + 'a' ) << "=" << print_expr( mig3, i ); 
          }
        }

        std::cout << "[expressions] " << ss.str() << std::endl;
        return ss.str();
      }

      void enumerate_m3ig( const kitty::dynamic_truth_table& tt )
      {
        bsat_wrapper solver;
        spec spec;
        also::mig3 mig3;

        auto copy = tt;
        if( copy.num_vars()  < 3 )
        {
          spec[0] = kitty::extend_to( copy, 3 );
        }
        else
        {
          spec[0] = tt;
        }

        also::mig_three_encoder encoder( solver );
        
        int nr_solutions = 0;

        while( also::next_solution( spec, mig3, solver, encoder ) == success )
        {
          print_all_expr( spec, mig3 );
          nr_solutions++;
        }

        std::cout << "There are " << nr_solutions << " solutions found." << std::endl;

      }

    protected:
      void execute()
      {
        auto& opt = store<optimum_network>().current();

        bsat_wrapper solver;
        spec spec;
        also::mig3 mig3;

        auto copy = opt.function;
        if( copy.num_vars()  < 3 )
        {
          spec[0] = kitty::extend_to( copy, 3 );
        }
        else
        {
          spec[0] = copy;
        }

        also::mig_three_encoder encoder( solver );

        if( is_set( "cegar" ) )
        {
          if ( also::mig_three_cegar_synthesize( spec, mig3, solver, encoder ) == success )
          {
            print_all_expr( spec, mig3 );
          }
        }
        else if( is_set( "enumerate" ) )
        {
          enumerate_m3ig( copy );
        }
        else
        {
          if ( also::mig_three_synthesize( spec, mig3, solver, encoder ) == success )
          {
            print_all_expr( spec, mig3 );
          }
        }
      }

  };

  ALICE_ADD_COMMAND( exact_m3ig, "Exact synthesis" )
}

#endif
