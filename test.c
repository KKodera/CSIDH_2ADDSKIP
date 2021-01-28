#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "steps.h"
#include "csidh.h"
#include "primes.h"

int main()
{
  private_key priv_alice, priv_bob;
  public_key pub_alice, pub_bob;
  public_key shared_alice, shared_bob;
  bool ok;

  for (long long bs = 0;bs <= 64;bs += 2) {
    for (long long gs = 0;;++gs) {
      if (!gs) if (bs) continue;
      if (!bs) if (gs) break;
      if (2*bs*gs > (primes[primes_num-1]-1)/2) break;
      if (gs > 4*bs) continue;
      if (bs > 4*gs) continue;

      printf("trying alice bs=%lld gs=%lld, bob bs=0 gs=0\n",bs,gs);
      fflush(stdout);

      steps_override(bs,gs);
    
      csidh_private(&priv_alice);
      ok = csidh(&pub_alice, &base, &priv_alice);
      assert(ok);
    
      steps_override(0,0);

      csidh_private(&priv_bob);
      ok = csidh(&pub_bob, &base, &priv_bob);
      assert(ok);
    
      ok = csidh(&shared_bob, &pub_alice, &priv_bob);
      assert(ok);

      steps_override(bs,gs);
    
      ok = csidh(&shared_alice, &pub_bob, &priv_alice);
      assert(ok);
  
      assert(!memcmp(&shared_alice, &shared_bob, sizeof(public_key)));
    }
  }

  return 0;
}
