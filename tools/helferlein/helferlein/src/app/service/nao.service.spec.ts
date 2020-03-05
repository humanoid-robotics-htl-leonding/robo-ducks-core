import { TestBed } from '@angular/core/testing';

import { NaoService } from './nao.service';

describe('NaoService', () => {
  beforeEach(() => TestBed.configureTestingModule({}));

  it('should be created', () => {
    const service: NaoService = TestBed.get(NaoService);
    expect(service).toBeTruthy();
  });
});
