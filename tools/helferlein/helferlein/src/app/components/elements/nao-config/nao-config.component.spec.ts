import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaoConfigComponent } from './nao-config.component';

describe('NaoConfigComponent', () => {
  let component: NaoConfigComponent;
  let fixture: ComponentFixture<NaoConfigComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NaoConfigComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaoConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
